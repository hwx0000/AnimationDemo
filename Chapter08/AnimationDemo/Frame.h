#ifndef _H_FRAME_
#define _H_FRAME_

// 这种模板的写法之前看的不多, 之前模板的<>里必须填类型
// 而这种写法把T类型specialized为unsigned int, 而且这里还加了具体类型的变量N
// 此时的<>里必须填unsigned int类型的值
template<unsigned int N>
class Frame
{
public:
	float mValue[N];// 这个Property由几个float组成
	float mIn[N];
	float mOut[N];
	float mTime;
};

// 三种specialized Frame
typedef Frame<1> ScalarFrame;
typedef Frame<3> VectorFrame;
typedef Frame<4> QuaternionFrame;

#endif
