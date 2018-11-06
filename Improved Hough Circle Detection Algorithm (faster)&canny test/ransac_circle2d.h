#ifndef RANSAC_CIRCLE2D_H_
#define RANSAC_CIRCLE2D_H_

#include "ransac2d.h"

namespace sac
{
	class ransacModelCircle2D : public RansacModel
	{
	public:

		using RansacModel::input_;
		using RansacModel::indices_;
		using RansacModel::error_sqr_dists_;

		typedef RansacModel::PointCloud PointCloud;

		ransacModelCircle2D(const PointCloud &cloud, double threshold, int max_iteration)
			:RansacModel(cloud, threshold, max_iteration){}

		ransacModelCircle2D(const PointCloud &cloud, const std::vector<int>&indices, double threshold, int max_iteration)
			:RansacModel(cloud, indices, threshold, max_iteration){}
			
		ransacModelCircle2D(){ m_dSpecificRadius = 0; m_dDiffRatio = 0.2; m_vMultiRadius.resize(0); }
		~ransacModelCircle2D(){};

		modelType getModelType() const { return MODEL_CIRCLE2D; }

		bool isGoodSample(const std::vector<int>& samples) const;

		bool computeModelCoefficients(const std::vector<int> &samples, ModelCoefficient &model_coefficient);

		int countWithinDistance(const ModelCoefficient model_coefficients, const double threshold);

		void selectWithinDistance(const ModelCoefficient model_coefficients, const double threshold, std::vector<int> &inliers);

		/**
		* \Brief: compute model of 2D circle with 3 parameters
		* \Details: first 2 parameters represent the center and the last one represent the radius
		*/
		bool computeModel();

		/**
		* \Brief: use this function when we want to detect circle of specific radius
		* \Details: when use function, we detect the circle of the radius set here with a threshold of 10%.
		*/
		inline void setSpecificRadius(double radius, double diffRatio) { m_dSpecificRadius = radius; m_dDiffRatio = diffRatio; }
		
		inline void setMultiRadius(std::vector<double> multiRaius, double diffRatio)
		{
			m_vMultiRadius.resize(multiRaius.size());

			for (size_t i = 0; i < multiRaius.size(); i++)
				m_vMultiRadius[i] = multiRaius[i];

			m_dDiffRatio = diffRatio;
		}
		
	private:
		double m_dSpecificRadius;
		double m_dDiffRatio;
		
		std::vector<double> m_vMultiRadius;
	};

}

#endif


