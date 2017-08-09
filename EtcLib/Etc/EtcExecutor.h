
#pragma once

#include <chrono>

#include "EtcImage.h"

namespace Etc {

	class Executor
	{
	public:
		using Milliseconds = std::chrono::milliseconds;

		Executor(Image& image);

		Image::EncodingStatus Encode(Image::Format a_format, ErrorMetric a_errormetric, float a_fEffort,
			unsigned int a_uiJobs, unsigned int a_uiMaxJobs);

		inline Milliseconds GetEncodingTime(void)
		{
			return m_msEncodeTime;
		}
	private:

		// stats
		Milliseconds m_msEncodeTime = Milliseconds::zero();
		Image& m_image;
		float m_fEffort = 0.0f;
	public:
		bool m_bVerboseOutput = false;

		friend class Image;
	};

} // namespace Etc
