#ifndef VEC6_H_
#define VEC6_H_

struct Vec6
{
    Vec6()=default;
    Vec6(const Vec6 &)=default;
    Vec6(float _r, float _g, float _b, float _x, float _y, float _z) : r(_r), g(_g), b(_b), x(_x), y(_y), z(_z){}

//    void operator +=(const Vec6 &_rhs)
//    {
//        x+=_rhs.x;
//        y+=_rhs.y;
//        z+=_rhs.z;
//    }

//    //this lets us compare 2 Vec3's together, using a floating point comparison range
//    #define Epsilon 0.0001f
//    #define FCompare( a,b ) \
//        ( ((a)-Epsilon)<(b) && ((a)+Epsilon>(b)))
//    bool operator ==(const Vec3 &_rhs)
//    {
//        return FCompare(x,_rhs.x) && FCompare(y,_rhs.y) && FCompare(z,_rhs.z);
//    }


    float r = 0;
    float g = 0;
    float b = 0;
    float x=0.0f;
    float y=0.0f;
    float z=0.0f;
};

#endif // VEC_6_H
