#include <load-gltf/load-gltf.hpp>

#include <iostream>

int main()
try
{
	lg::Gltf gltf = lg::loadGltf("{}}");
	return 0;
}
catch (std::exception const& ex)
{
	std::cerr << "Unexpected exception: " << ex.what() << std::endl;
	return 1;
}
