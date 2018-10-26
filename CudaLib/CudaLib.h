#pragma once
#include "cuda_runtime.h"
#include "device_launch_parameters.h"

namespace Procon2018 {
namespace CudaLib {


const int MAX_H = 12, MAX_W = 12;


int CalcAreaPoint(int gc[][MAX_H][MAX_W]);

//gc: [caseSize][MAX_H][MAX_W]
__global__ int d_CalcAreaPoint(int ***gc);


}
}