/*
 * Copyright 2015 The Etc2Comp Authors.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "EtcConfig.h"
#include "Etc.h"
#include "EtcThreadedExecutor.h"
#include "EtcFilter.h"

#include <cstring>

namespace Etc
{
	// ----------------------------------------------------------------------------------------------------
	// C-style inteface to the encoder
	//
	void Encode(float *a_pafSourceRGBA,
				unsigned int a_uiSourceWidth, 
				unsigned int a_uiSourceHeight,
				Image::Format a_format,
				ErrorMetric a_eErrMetric,
				float a_fEffort,
				unsigned int a_uiJobs,
				unsigned int a_uiMaxJobs,
				unsigned char **a_ppaucEncodingBits,
				unsigned int *a_puiEncodingBitsBytes,
				unsigned int *a_puiExtendedWidth,
				unsigned int *a_puiExtendedHeight, 
				int *a_piEncodingTime_ms, bool a_bVerboseOutput)
	{

		Image image(a_pafSourceRGBA, a_uiSourceWidth,
					a_uiSourceHeight,
					a_eErrMetric);
		ThreadedExecutor executor(image);
		executor.m_bVerboseOutput = a_bVerboseOutput;
		auto result = TimeEncode(executor, a_format, a_eErrMetric, a_fEffort, a_uiJobs, a_uiMaxJobs);

		*a_ppaucEncodingBits = executor.GetEncodingBits();
		*a_puiEncodingBitsBytes = executor.GetEncodingBitsBytes();
		*a_puiExtendedWidth = image.GetExtendedWidth();
		*a_puiExtendedHeight = image.GetExtendedHeight();
		*a_piEncodingTime_ms = result.m_msEncodeTime.count();
	}

	void EncodeMipmaps(float *a_pafSourceRGBA,
		unsigned int a_uiSourceWidth,
		unsigned int a_uiSourceHeight,
		Image::Format a_format,
		ErrorMetric a_eErrMetric,
		float a_fEffort,
		unsigned int a_uiJobs,
		unsigned int a_uiMaxJobs,
		unsigned int a_uiMaxMipmaps,
		unsigned int a_uiMipFilterFlags,
		RawImage* a_pMipmapImages,
		int *a_piEncodingTime_ms, 
		bool a_bVerboseOutput)
	{
		auto mipWidth = a_uiSourceWidth;
		auto mipHeight = a_uiSourceHeight;
		int totalEncodingTime = 0;
		for(unsigned int mip = 0; mip < a_uiMaxMipmaps && mipWidth >= 1 && mipHeight >= 1; mip++)
		{
			float* pImageData = nullptr;
			float* pMipImage = nullptr;

			if(mip == 0)
			{
				pImageData = a_pafSourceRGBA;
			}
			else
			{
				pMipImage = new float[mipWidth*mipHeight*4];
				if(FilterTwoPass(a_pafSourceRGBA, a_uiSourceWidth, a_uiSourceHeight, pMipImage, mipWidth, mipHeight, a_uiMipFilterFlags, Etc::FilterLanczos3) )
				{
					pImageData = pMipImage;
				}
			}

			if ( pImageData )
			{
			
				Image image(pImageData, mipWidth, mipHeight,	a_eErrMetric);
				ThreadedExecutor executor(image);
				executor.m_bVerboseOutput = a_bVerboseOutput;
				auto const result = TimeEncode(executor, a_format, a_eErrMetric, a_fEffort, a_uiJobs, a_uiMaxJobs);

				a_pMipmapImages[mip].paucEncodingBits = std::shared_ptr<unsigned char>(executor.GetEncodingBits(), [](unsigned char *p) { delete[] p; });
				a_pMipmapImages[mip].uiEncodingBitsBytes = executor.GetEncodingBitsBytes();
			a_pMipmapImages[mip].uiExtendedWidth = image.GetExtendedWidth();
			a_pMipmapImages[mip].uiExtendedHeight = image.GetExtendedHeight();

				totalEncodingTime += result.m_msEncodeTime.count();
			}

			if(pMipImage)
			{
				delete[] pMipImage;
			}

			if (!pImageData)
			{
				break;
			}

			mipWidth >>= 1;
			mipHeight >>= 1;
		}

		*a_piEncodingTime_ms = totalEncodingTime;
	}


	// ----------------------------------------------------------------------------------------------------
	//

}
