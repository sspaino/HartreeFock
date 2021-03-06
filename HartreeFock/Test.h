#pragma once

#include "ChemUtils.h"
#include "Basis.h"


class Test
{
public:
	Test(const std::string& basisFile = "sto3g.txt");

	static void OutputMatricesForAtom(const std::string& atomName, const std::string& basisSetName, const std::string& fileName);
	static void OutputMatrices(Systems::Molecule& molecule, std::ofstream& file, const std::string& sfileName = "", const std::string& tfileName = "", const std::string& vfileName = "", const std::string& erifileName = "", bool useDIIS = false, const double expectedNucEnergy = 0);

	static void CheckDifferences(const Eigen::MatrixXd& matrix, const std::string& matrixFileName, std::ofstream& file);
	static void CheckDifferences(const GaussianIntegrals::IntegralsRepository& repo, const std::string& eriFileName, std::ofstream& file);

	void TestWater(const std::string& fileName, const std::string& sfileName = "", const std::string& tfileName = "", const std::string& vfileName = "", const std::string& erifileName = "", bool useDIIS = false);
	void TestMethane(const std::string& fileName, const std::string& sfileName = "", const std::string& tfileName = "", const std::string& vfileName = "", const std::string& erifileName = "", bool useDIIS = false);

protected:
	static void OutputMatrix(const Eigen::MatrixXd& matrix, std::ofstream& file);

	Chemistry::Basis basis;
};

