#pragma once
#include <d3d11_4.h>

class Block
{
public:
	Block(ID3D11DeviceContext1* context, float x, float y, float z);
	~Block();
};

