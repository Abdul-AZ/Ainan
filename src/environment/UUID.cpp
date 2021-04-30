#include "UUID.h"

namespace Ainan {

	UUID::UUID()
	{
		//initlizze UUID as a Nil UUID
		Data.fill(0);
	}

	void UUID::Generate(std::mt19937& randomEngine)
	{
		std::uniform_int_distribution dest(0, 255);
		for (size_t i = 0; i < Data.size(); i++)
		{
			Data[i] = (uint8_t)dest(randomEngine);
		}

		//make generated uuid version 4 compatible
		Data[6] = 0x40 | (Data[6] & 0xf);
		Data[8] = 0x80 | (Data[8] & 0x3f);
	}

	void UUID::FromString(const std::string& str)
	{
		//remove hyphens
		std::string plainUUIDstr = str;
		plainUUIDstr.erase(std::remove(plainUUIDstr.begin(), plainUUIDstr.end(), '-'));
		plainUUIDstr.resize(32);
		
		//convert from hex to binary and store result in Data
		for (size_t i = 0; i < plainUUIDstr.length(); i += 2) 
		{
			std::string byteString = plainUUIDstr.substr(i, 2);
			uint8_t byte = (uint8_t)std::strtol(byteString.c_str(), NULL, 16);
			Data[i / 2] = byte;
		}
	}

	std::string UUID::GetAsUUIDString() const
	{
		std::string str;
		for (size_t i = 0; i < Data.size(); i++)
		{
			char digit[3];
			sprintf(digit, "%02x", Data[i]);
			str.append(digit);
		}

		//insert hyphens so that the string shows the standard reprisentation of a UUID
		str.insert(str.begin() + 8, '-');
		str.insert(str.begin() + 12 + 1, '-');
		str.insert(str.begin() + 16 + 2, '-');
		str.insert(str.begin() + 20 + 3, '-');

		return str;
	}

	bool UUID::operator==(const UUID& other)
	{
		for (size_t i = 0; i < Data.size(); i++)
		{
			if (Data[i] != other.Data[i])
				return false;
		}

		return true;
	}

	bool UUID::operator!=(const UUID& other)
	{
		return !((*this) ==  other);
	}
}