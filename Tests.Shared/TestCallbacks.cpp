#include "stdafx.h"
#include <Shared/Action.hpp>

// Testing callbacks
static int callCounter = 0;
void TestCallback(void)
{
	callCounter++;
}
class TestClass
{
public:
	void TestCallback()
	{
		classCallCounter++;
	}
	int classCallCounter = 0;
};

Test("Delegate.LambdaRValue")
{
	Delegate<int> dv;
	int testCounter = 0;
	dv.AddLambda([&](int arg)
	{
		testCounter = arg;
	});

	dv.Call(101);
	TestEnsure(testCounter == 101);
}

Test("Delegate.Call")
{
	TestClass tc;
	callCounter = 0;

	auto lambda = []()
	{
		callCounter++;
	};

	Delegate<> dv;
	dv.Add(&TestCallback);
	dv.Add(&tc, &TestClass::TestCallback);
	dv.AddLambda(lambda);
	dv.Call();

	TestEnsure(callCounter == 2);
	TestEnsure(tc.classCallCounter == 1);

	dv.Clear();

	// Ensure state persist after clear
	dv.Call();
	TestEnsure(callCounter == 2);
	TestEnsure(tc.classCallCounter == 1);

	// Ensure state after removeing callbacks manually
	dv.Add(&TestCallback);
	dv.Add(&tc, &TestClass::TestCallback);
	DelegateHandle dh = dv.AddLambda(lambda);
	dv.Remove(dh);
	dv.Remove(&TestCallback);
	dv.Remove(&tc, &TestClass::TestCallback);
	dv.Call();
	TestEnsure(callCounter == 2);
	TestEnsure(tc.classCallCounter == 1);
}

Test("Action.Assignment")
{
	Action<> a;
	TestEnsure(!a.IsBound());

	// Static binding
	callCounter = 0;
	a = []()
	{
		callCounter++;
	};
	a.Call();
	TestEnsure(callCounter == 1);

	// Lamba binding
	int localCallCounter = 0;
	Action<> b = [&]()
	{
		localCallCounter++;
	};
	b.Call();
	TestEnsure(localCallCounter == 1);
}

float ConstantValue()
{
	return 2.0f;
}
Test("Action.Property")
{
	float localValue = 0.0f;
	Property<float> testProp;
	testProp.Set = [&](float v)
	{
		localValue = v;
	};
	testProp.Get = [&]()
	{
		return localValue;
	};

	testProp = 10.0f;
	TestEnsure(testProp == 10.0f);

	testProp = testProp + 10.0f;
	TestEnsure(testProp > 15.0f);

	testProp.Get = &ConstantValue;
	TestEnsure(testProp == 2.0f);

	class FloatWrapper
	{
	public:
		float Get()
		{
			return val;
		}
		void Set(float v)
		{
			val = v;
		}
		float val = 0.0f;
	};
	FloatWrapper o;
	testProp.Get.Bind(&o, &FloatWrapper::Get);
	testProp.Set.Bind(&o, &FloatWrapper::Set);

	TestEnsure(testProp == 0.0f);
	o.val = 1.0f;
	TestEnsure(testProp == 1.0f);
	testProp = 4.0f;
	TestEnsure(o.val == 4.0f);
}


Test("Action.InlineProperty")
{
	class Test
	{
	private:
		float privateFloat = 0.0f;
		Test* privatePointer = this;
	public:
		Property<float> MyProperty 
		{
			get
			{
				return privateFloat;
			}
			set
			{
				privateFloat = value;
			}
		};

		Property<Test*> InnerClassPointer
		{
			get
			{
				return privatePointer;
			}
			set
			{
			}
		};
	};

	Test a;
	a.MyProperty = 10.0f;
	TestEnsure(a.MyProperty == 10.0f);
	TestEnsure(a.InnerClassPointer == &a);
	a.InnerClassPointer = nullptr;
	TestEnsure(a.InnerClassPointer == &a);

}