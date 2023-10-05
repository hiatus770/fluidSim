// Define some operators for vector2 that are missing such as + - * /
Vector2 operator+(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x + v2.x, v1.y + v2.y};
}
Vector2 operator-(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x - v2.x, v1.y - v2.y};
}
Vector2 operator*(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x * v2.x, v1.y * v2.y};
}
Vector2 operator/(const Vector2 &v1, const Vector2 &v2)
{
    return {v1.x / v2.x, v1.y / v2.y};
}
Vector2 operator*(const Vector2 &v1, const double &v2)
{
    return {v1.x * v2, v1.y * v2};
}
Vector2 operator/(const Vector2 &v1, const double &v2)
{
    return {v1.x / v2, v1.y / v2};
}

