#pragma once

struct AABB {
	Vector3 min;
	Vector3 max;
};

bool IsCollisionAABB(const AABB& a, const AABB& b);