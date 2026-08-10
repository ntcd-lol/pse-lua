// Portal Solver Editor SDK

#pragma once

#ifdef __cplusplus

#include <cstring>
#include <stdint.h>

#include "pse/pse.h"

namespace pse
{
    namespace Register 
    {
		// Bit
		inline void setBit(PseRegister& outRegister, const uint8_t inOffset, const bool inValue)
		{
			outRegister = (outRegister & ~(1 << inOffset)) | (static_cast<PseRegister>(inValue) << inOffset);
		}
		
		inline bool getBit(const PseRegister inRegister, const uint8_t inOffset)
		{
			return (inRegister & (1 << inOffset)) != 0;
		}
		
		inline void updateBit(const bool toRegister, PseRegister& outRegister, const uint8_t inOffset, bool& outValue)
		{
			if(toRegister) setBit(outRegister, inOffset, outValue);
			else outValue = getBit(outRegister, inOffset);
		}
		
		// Int
		template<typename T>
		T getInt(const PseRegister inRegister, const uint8_t inOffset)
		{
	        constexpr uint8_t BitSize = sizeof(T) * 8;
			static_assert(sizeof(T) * 8 <= 32, "T is big!");
	        constexpr uint32_t Mask = (BitSize == 32) ? 0xFFFFFFFF : ((1U << BitSize) - 1);
			return static_cast<T>((inRegister >> inOffset) & Mask);
		}
		
		template<typename T>
		void setInt(PseRegister& outRegister, const uint8_t inOffset, const T inValue)
		{
	        constexpr uint8_t BitSize = sizeof(T) * 8;
			static_assert(sizeof(T) * 8 <= 32, "T is big!");
	        constexpr uint32_t Mask = (BitSize == 32) ? 0xFFFFFFFF : ((1U << BitSize) - 1);
			outRegister = (outRegister & ~(Mask << inOffset)) | ((static_cast<PseRegister>(inValue) & Mask) << inOffset);
		}
		
		template<typename T, typename Q>
		void updateInt(const bool toRegister, PseRegister& outRegister, const uint8_t inOffset, Q& outValue)
		{
			if(toRegister) setInt<T>(outRegister, inOffset, static_cast<T>(outValue));
			else outValue = static_cast<Q>(getInt<T>(outRegister, inOffset));
		}
		
		// BitAs
		template<typename T, uint8_t bitCount>
		T getBitsAs(const PseRegister inRegister, const uint8_t inOffset)
		{
			static_assert(bitCount > 0 && bitCount <= 32, "bitCount must be between 1 and 32!");
	        constexpr uint32_t Mask = (bitCount == 32) ? 0xFFFFFFFF : ((1U << bitCount) - 1);
			return static_cast<T>((static_cast<PseRegister>(inRegister) >> inOffset) & Mask);
		}
		
		template<typename T, uint8_t bitCount>
		void setBitsAs(PseRegister& outRegister, const uint8_t inOffset, const T inValue)
		{
			static_assert(bitCount > 0 && bitCount <= 32, "bitCount must be between 1 and 32!");
	        constexpr uint32_t Mask = (bitCount == 32) ? 0xFFFFFFFF : ((1U << bitCount) - 1);
			outRegister = static_cast<PseRegister>((static_cast<PseRegister>(outRegister) & ~(Mask << inOffset)) | ((static_cast<PseRegister>(inValue) & Mask) << inOffset));
		}
		
		template<typename T, uint8_t bitCount, typename Q>
		void updateBitsAs(const bool toRegister, PseRegister& outRegister, const uint8_t inOffset, Q& outValue)
		{
			if(toRegister) setBitsAs<T, bitCount>(outRegister, inOffset, static_cast<T>(outValue));
			else outValue = static_cast<Q>(getBitsAs<T, bitCount>(outRegister, inOffset));
		}
    }
	
    namespace Data
    {
    	template<uint8_t offset, typename T>
        void set(PseData& outData, const T& inValue)
        {
        	static_assert(offset + sizeof(T) <= 60, "offset + sizeof(T) must be between 0 and 60!");
            std::memcpy(&outData.data[offset], &inValue, sizeof(T));
        }
        
    	template<uint8_t offset, typename T>
        T get(const PseData& inData)
        {
        	static_assert(offset + sizeof(T) <= 60, "offset + sizeof(T) must be between 0 and 60!");
            T result;
            std::memcpy(&result, &inData.data[offset], sizeof(T));
            return result;
        }
    }
	
    namespace Events
    {
		//  1 - Событие успешно извлечено и обработано встроенным колбэком
        //  0 - Событие извлечено, но не распознано SDK (отдано пользователю в outData)
        // -1 - Буфер событий пуст
        inline int32_t poll(PseData& outData)
        {
            PseData data{};
            
            if(pseEventBufferTryGet(&data) == 0)
				return -1;
            
			switch(data.header)
			{
			default: break;
			case PSE_EVENT_ELEMENT_CHANGED:
			    {
                    PseCallback callback = pse::Data::get<8, PseCallback>(data);
                    if(callback == 0) return false;
					
                    PseState state = pse::Data::get<16, PseState>(data);
                    auto pfnCallback = reinterpret_cast<void(*)(PseState)>(callback);
                    pfnCallback(state);
					
                    return 1;
                }
				break;
			}
			
            return 0;
        }
		
        inline int32_t poll()
        {
            PseData data{};
            return poll(data);
        }
	}
	
    namespace Quat
    {
    	inline PseQuat fromRadians(const float pitch, const float yaw, const float roll)
    	{
    		const float pitchHalf = pitch * 0.5f;
    		const float yawHalf   = yaw   * 0.5f;
    		const float rollHalf  = roll  * 0.5f;
    		
    		const float sp = std::sin(pitchHalf);
    		const float cp = std::cos(pitchHalf);
    		
    		const float sy = std::sin(yawHalf);
    		const float cy = std::cos(yawHalf);
    		
    		const float sr = std::sin(rollHalf);
    		const float cr = std::cos(rollHalf);
    		
    		PseQuat quat{};
    		quat.x = cr * sp * sy + sr * cp * cy;
    		quat.y = cr * sp * cy - sr * cp * sy;
    		quat.z = cr * cp * sy - sr * sp * cy;
    		quat.w = cr * cp * cy + sr * sp * sy;
    		
    		return quat;
    	}
    	
    	inline PseQuat fromDegrees(const float pitch, const float yaw, const float roll)
    	{
    		constexpr float degreesToRadians = 3.14159265358979323846f / 180.0f;
    		return fromRadians(pitch * degreesToRadians, yaw * degreesToRadians, roll * degreesToRadians);
    	}
    }
}

#endif
