#include "../src/Kernel.h"
#include "../src/LogicalCube.h"
#include "../src/Layer.h"
#include "../src/config.h"
#include "../src/Connector.h"
#include "../src/bridges/ReLUBridge.h"
#include "test_types.h"
#include "gtest/gtest.h"
#include "glog/logging.h"
#include <iostream>
#include <fstream>
#include <assert.h>
#include <cmath>
#include <cstring>

template <typename TypeParam>
class ReLUBridgeTest : public ::testing::Test {
 public:
  typedef typename TypeParam::T T;	
  ReLUBridgeTest(){
  	data1 = new LogicalCube<T, Layout_CRDB>(iR, iC, iD, mB);
    grad1 = new LogicalCube<T, Layout_CRDB>(iR, iC, iD, mB);
    
    data2 = new LogicalCube<T, Layout_CRDB>(iR, iC, iD, mB);
    grad2 = new LogicalCube<T, Layout_CRDB> (iR, iC, iD, mB);

    layer1 = new Layer<T, Layout_CRDB>(data1, grad1);
    layer2 = new Layer<T, Layout_CRDB>(data2, grad2);
    
    ReLUBridge_ = new ReLUBridge<T, Layout_CRDB, T, Layout_CRDB>(layer1, layer2);
   } 

  	virtual ~ReLUBridgeTest() { delete ReLUBridge_; delete layer1; delete layer2;}
    ReLUBridge<T, Layout_CRDB, T, Layout_CRDB>* ReLUBridge_;

  	LogicalCube<T, Layout_CRDB>* data1;
    LogicalCube<T, Layout_CRDB>* grad1;
    
    LogicalCube<T, Layout_CRDB>* data2;
    LogicalCube<T, Layout_CRDB>* grad2;

    Layer<T, Layout_CRDB>* layer1;
    Layer<T, Layout_CRDB>* layer2;

    const int mB = 2;
    const int iD = 3;
    const int iR = 10;
    const int iC = 10;
};

typedef ::testing::Types<FloatCRDB> DataTypes;

TYPED_TEST_CASE(ReLUBridgeTest, DataTypes);

//openblas_set_num_threads -- undefined reference -- currently disabled
TYPED_TEST(ReLUBridgeTest, TestInitialization){
  EXPECT_TRUE(this->ReLUBridge_);
  EXPECT_TRUE(this->layer1);
  EXPECT_TRUE(this->layer2);
}

TYPED_TEST(ReLUBridgeTest, TestForward){
    srand(1);
	typedef typename TypeParam::T T;
	for(int i=0;i<this->iR*this->iC*this->iD*this->mB;i++){
        this->data1->p_data[i] = rand()%2 - rand()%2;
    }

    int oR = this->iR;
    int oC = this->iC;
    LogicalCube<T, Layout_CRDB>* out_expected = new LogicalCube<T, Layout_CRDB>(oR, oC, this->iD, this->mB);

    this->ReLUBridge_->forward();

    std::fstream expected_output("relu_forward.txt", std::ios_base::in);
    
    T output;
    int idx = 0;
    if (expected_output.is_open()) {
        expected_output >> output;
        while (!expected_output.eof()) {
            EXPECT_NEAR(this->data2->p_data[idx], output, EPS);
            expected_output >> output;
            idx++;
        }
    }
    expected_output.close();
}


TYPED_TEST(ReLUBridgeTest, TestBackward){
    typedef typename TypeParam::T T;
    
    srand(1);
    for(int i=0;i<this->iR*this->iC*this->iD*this->mB;i++){
        this->data1->p_data[i] = rand()%2 - rand()%2;
        this->grad1->p_data[i] = 1;
    }
    
    int oR = this->iR;
    int oC = this->iC;

    for(int i=0;i<oR*oC*this->iD*this->mB;i++){
        this->data2->p_data[i] = 1;
        this->grad2->p_data[i] = i;
    }

    this->ReLUBridge_->forward();
    
    cube->logical_print();    
    this->ReLUBridge_->backward();

    std::fstream expected_output("relu_backward.txt", std::ios_base::in);
    
    T output;
    int idx = 0;
    if (expected_output.is_open()) {
        expected_output >> output;
        while (!expected_output.eof()) {
            EXPECT_NEAR(this->grad1->p_data[idx], output, EPS);
            expected_output >> output;
            idx++;
        }
    }
    expected_output.close();   
}
