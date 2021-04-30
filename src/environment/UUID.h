#pragma once

namespace Ainan {

	class UUID
	{
	public:
		//initlize UUID as nil
		UUID();

		//constructs the UUID from a standard version 4 string representation
		void FromString(const std::string& str);

		//generates a new UUID
		void Generate(std::mt19937& randomEngine);

		//gets the standard version 4 string representation of the UUID
		std::string GetAsUUIDString() const;

		//comparison operators
		bool operator==(const UUID& other);
		bool operator!=(const UUID& other);

	public:
		std::array<uint8_t, 16> Data;
	};
}