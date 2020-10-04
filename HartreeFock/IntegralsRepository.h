#pragma once

#include "MathUtils.h"
#include "GaussianOverlap.h"
#include "GaussianKinetic.h"
#include "GaussianNuclear.h"
#include "GaussianTwoElectrons.h"
#include "BoysFunctions.h"

#include <map>
#include <tuple>
#include <valarray>

#include "Molecule.h"
#include "ContractedGaussianOrbital.h"

namespace GaussianIntegrals {

	class IntegralsRepository
	{
	public:
		Systems::Molecule* m_Molecule;
	protected:
		std::map< double, BoysFunctions > boysFunctions;


		std::map < std::tuple<unsigned int, unsigned int, double, double>, GaussianOverlap> overlapIntegralsMap;
		std::map < std::tuple<unsigned int, unsigned int, double, double >, GaussianKinetic> kineticIntegralsMap;

		std::map < std::tuple<unsigned int, unsigned int, unsigned int, double, double >, GaussianNuclear > nuclearVerticalIntegralsMap;
		std::map < std::tuple<unsigned int, unsigned int, unsigned int>, GaussianNuclear> nuclearIntegralsContractedMap;
		
		std::map < std::tuple<unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, double, double, double, double>, GaussianTwoElectrons> electronElectronIntegralsVerticalAndTransferMap;
		std::map < std::tuple<unsigned int, unsigned int, unsigned int, unsigned int>, GaussianTwoElectrons> electronElectronIntegralsContractedMap;
		std::valarray<double> electronElectronIntegrals;

	public:
		bool useLotsOfMemory;

		IntegralsRepository(Systems::Molecule *molecule = nullptr);
		~IntegralsRepository();

		void Reset(Systems::Molecule* molecule = nullptr);

		const BoysFunctions& getBoysFunctions(unsigned int L, double T);


		double getOverlap(const Orbitals::ContractedGaussianOrbital& orbital1, const Orbitals::ContractedGaussianOrbital& orbital2, bool extendForKinetic = true);
		double getOverlap(const Orbitals::GaussianOrbital& gaussian1, const Orbitals::GaussianOrbital& gaussian2, bool extendForKinetic = true);

		double getKinetic(const Orbitals::ContractedGaussianOrbital& orbital1, const Orbitals::ContractedGaussianOrbital& orbital2);
		double getKinetic(const Orbitals::GaussianOrbital& gaussian1, const Orbitals::GaussianOrbital& gaussian2);


		double getNuclear(const Systems::Atom& atom, const Orbitals::ContractedGaussianOrbital* orbital1, const Orbitals::ContractedGaussianOrbital* orbital2);


		double getElectronElectron(const Orbitals::ContractedGaussianOrbital* orbital1, const Orbitals::ContractedGaussianOrbital* orbital2, const Orbitals::ContractedGaussianOrbital* orbital3, const Orbitals::ContractedGaussianOrbital* orbital4);
		const GaussianTwoElectrons& getElectronElectronVerticalAndTransfer(const Orbitals::GaussianOrbital* orbital1, const Orbitals::GaussianOrbital* orbital2, const Orbitals::GaussianOrbital* orbital3, const Orbitals::GaussianOrbital* orbital4, bool& swapped);



		Systems::Molecule* getMolecule() const { return m_Molecule; }


		void ClearMatricesMaps() 
		{
			overlapIntegralsMap.clear();
			kineticIntegralsMap.clear();

			nuclearVerticalIntegralsMap.clear();
			nuclearIntegralsContractedMap.clear();
		}

		void ClearElectronElectronIntermediaries()
		{
			electronElectronIntegralsVerticalAndTransferMap.clear();
		}

		void ClearElectronElectronMaps()
		{
			ClearElectronElectronIntermediaries();

			electronElectronIntegralsContractedMap.clear();
		}

		void ClearAllMaps()
		{
			ClearMatricesMaps();
			ClearElectronElectronMaps();
			boysFunctions.clear();
		}
	protected:
		const GaussianNuclear& getNuclearVertical(const Systems::Atom& atom, const Orbitals::GaussianOrbital& gaussian1, const Orbitals::GaussianOrbital& gaussian2);

		inline void CalculateElectronElectronIntegrals23(int i, const Orbitals::ContractedGaussianOrbital& orb1);
		inline void CalculateElectronElectronIntegrals4(int i, int j, int k, long long int ij, const Orbitals::ContractedGaussianOrbital& orb1, const Orbitals::ContractedGaussianOrbital& orb2, const Orbitals::ContractedGaussianOrbital& orb3);
	
		inline static long long int GetTwoIndex(long long int i, long long int j)
		{
			return i * (i + 1ULL) / 2 + j;
		}
	
		inline static long long int GetElectronElectronIndex(int ind1, int ind2, int ind3, int ind4)
		{
			if (ind1 < ind2) std::swap(ind1, ind2);
			if (ind3 < ind4) std::swap(ind3, ind4);

			long long int ind12 = GetTwoIndex(ind1, ind2);
			long long int ind34 = GetTwoIndex(ind3, ind4);

			if (ind12 < ind34) std::swap(ind12, ind34);

			return GetTwoIndex(ind12, ind34);
		}

		template<class Orb> static void SwapOrbitals(Orb **orb1, Orb **orb2, Orb **orb3, Orb **orb4);
	public:
		void CalculateElectronElectronIntegrals();

		inline double getElectronElectron(int orbital1, int orbital2, int orbital3, int orbital4) const
		{
			return electronElectronIntegrals[GetElectronElectronIndex(orbital1, orbital2, orbital3, orbital4)];
		}
	};


	class MP2MolecularOrbitalsIntegralsRepository
	{
	public:
		MP2MolecularOrbitalsIntegralsRepository(const IntegralsRepository& repository) 
			: m_repo(repository)
		{
		}

		double getElectronElectron(int orbital1, int orbital2, int orbital3, int orbital4, const Eigen::MatrixXd& C)
		{
			const std::tuple<int, int, int, int> indTuple = std::make_tuple(orbital1, orbital2, orbital3, orbital4);
			if (m_fourthLevelIntegrals.find(indTuple) != m_fourthLevelIntegrals.end())
				return m_fourthLevelIntegrals.at(indTuple);

			// don't have it yet, compute it
			double result = 0;

			// The very slow method, gets the same results as the faster one, but I think they might be both wrong
			/*
			for (int i = 0; i < C.cols(); ++i)
			{
				double res1 = 0;
				for (int j = 0; j < C.cols(); ++j)
				{
					double res2 = 0;
					for (int k = 0; k < C.cols(); ++k)
					{
						double res3 = 0;
						for (int l = 0; l < C.cols(); ++l)
							res3 += C(l, orbital4) * m_repo.getElectronElectron(i, j, k, l);
						res2 += C(k, orbital3) * res3;
					}
					res1 += C(j, orbital2) * res2;
				}
				result += C(i, orbital1) * res1;
			}
			*/
			
			for (int mu = 0; mu < C.cols(); ++mu)
				result += C(mu, orbital1) * getElectronElectronThirdLevel(mu, orbital2, orbital3, orbital4, C);

			m_fourthLevelIntegrals[indTuple] = result;

			return result;
		}

	protected:

		double getElectronElectronFirstLevel(int orbital1, int orbital2, int orbital3, int orbital4, const Eigen::MatrixXd& C)
		{
			const std::tuple<int, int, int, int> indTuple = std::make_tuple(orbital1, orbital2, orbital3, orbital4);
			if (m_firstLevelIntegrals.find(indTuple) != m_firstLevelIntegrals.end())
				return m_firstLevelIntegrals.at(indTuple);

			// don't have it yet, compute it
			double result = 0;
			for (int s = 0; s < C.cols(); ++s)
				result += C(s, orbital4) * m_repo.getElectronElectron(orbital1, orbital2, orbital3, s);
			
			m_firstLevelIntegrals[indTuple] = result;
			
			return result;
		}

		double getElectronElectronSecondLevel(int orbital1, int orbital2, int orbital3, int orbital4, const Eigen::MatrixXd& C)
		{
			const std::tuple<int, int, int, int> indTuple = std::make_tuple(orbital1, orbital2, orbital3, orbital4);
			if (m_secondLevelIntegrals.find(indTuple) != m_secondLevelIntegrals.end())
				return m_secondLevelIntegrals.at(indTuple);

			// don't have it yet, compute it
			double result = 0;

			for (int l = 0; l < C.cols(); ++l)
				result += C(l, orbital3) * getElectronElectronFirstLevel(orbital1, orbital2, l, orbital4, C);
			
			m_secondLevelIntegrals[indTuple] = result;

			return result;
		}

		double getElectronElectronThirdLevel(int orbital1, int orbital2, int orbital3, int orbital4, const Eigen::MatrixXd& C)
		{
			const std::tuple<int, int, int, int> indTuple = std::make_tuple(orbital1, orbital2, orbital3, orbital4);
			if (m_thirdLevelIntegrals.find(indTuple) != m_thirdLevelIntegrals.end())
				return m_thirdLevelIntegrals.at(indTuple);

			// don't have it yet, compute it
			double result = 0;
			for (int nu = 0; nu < C.cols(); ++nu)
				result += C(nu, orbital2) * getElectronElectronSecondLevel(orbital1, nu, orbital3, orbital4, C);

			m_thirdLevelIntegrals[indTuple] = result;

			return result;
		}

		const IntegralsRepository& m_repo;
		std::map<std::tuple<int, int, int, int>, double> m_firstLevelIntegrals;
		std::map<std::tuple<int, int, int, int>, double> m_secondLevelIntegrals;
		std::map<std::tuple<int, int, int, int>, double> m_thirdLevelIntegrals;
		std::map<std::tuple<int, int, int, int>, double> m_fourthLevelIntegrals;
	};

}

