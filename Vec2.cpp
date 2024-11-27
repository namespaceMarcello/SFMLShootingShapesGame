#include "Vec2.h"
#include <math.h>
#include <iostream>


Vec2::Vec2()
{

}

Vec2::Vec2(float xin, float yin)
	: x(xin), y(yin)
{

}

bool Vec2::operator == (const Vec2& rhs) const
{
	return (x == rhs.x && y == rhs.y);
}

bool Vec2::operator != (const Vec2& rhs) const
{
	return !(x == rhs.x && y == rhs.y);
}

Vec2 Vec2::operator + (const Vec2& rhs) const
{
	return Vec2(x + rhs.x, y + rhs.y);
}

Vec2 Vec2::operator - (const Vec2& rhs) const
{
	return Vec2(x - rhs.x, y - rhs.y);
}

Vec2 Vec2::operator * (const float val) const
{
	return Vec2(x * val, y * val);
}

Vec2 Vec2::operator / (const float val) const
{
	return Vec2(x / val, y / val);
}

void Vec2::operator += (const Vec2& rhs) 
{
	x += rhs.x;
	y += rhs.y;
}

void Vec2::operator -= (const Vec2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

void Vec2::operator *= (const float val)
{
	x *= val;
	y *= val;
}

void Vec2::operator /= (const float val)
{
	x /= val;
	y /= val;
}

void Vec2::printVec() const
{
	std::cout << "Vec print:" << " " << x << ", " << y << std::endl;
}


float Vec2::length() const 
{

	return std::sqrt(x * x + y * y);
}

Vec2 Vec2::normalized(float desiredLength) const
{
	float currentLength = length();

	if (currentLength > std::numeric_limits<float>::epsilon())
	{
		float scale = desiredLength / currentLength;
		return Vec2(x * scale, y * scale);
	}

	return Vec2(desiredLength, 0); // Nel caso di vettore nullo, restituisci un vettore con la lunghezza desiderata sull'asse X
}


Vec2 Vec2::velocityForBullet(const Vec2& targetPosition, const float bulletSpeed) const
{

	return targetPosition * bulletSpeed;
}


