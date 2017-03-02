
#pragma once

class Test {
public:
    
    Test(int val) : mVal(val), mName("test") {}
    
    int getVal() const { return mVal; }
    
    ~Test() { mName.clear(); }
    
private:
    
    int mVal;
    std::string mName;
    
};

int randInt( int max ){
    auto r = rand() / (float)RAND_MAX;
    return r * max;
}