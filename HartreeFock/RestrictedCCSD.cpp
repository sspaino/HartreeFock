#include "stdafx.h"
#include "RestrictedCCSD.h"


namespace HartreeFock {

	RestrictedCCSD::RestrictedCCSD(int iterations)
		: RestrictedHartreeFock(iterations), m_spinOrbitalBasisIntegrals(nullptr)
	{

	}

	RestrictedCCSD::~RestrictedCCSD()
	{
		delete m_spinOrbitalBasisIntegrals;
	}

	void RestrictedCCSD::Init(Systems::Molecule* molecule)
	{
		RestrictedHartreeFock::Init(molecule);

		if (m_spinOrbitalBasisIntegrals) delete m_spinOrbitalBasisIntegrals;
		m_spinOrbitalBasisIntegrals = new GaussianIntegrals::CoupledClusterSpinOrbitalsElectronElectronIntegralsRepository(numberOfOrbitals);

		m_spinOrbitalBasisIntegrals->Compute(integralsRepository);
	}

}