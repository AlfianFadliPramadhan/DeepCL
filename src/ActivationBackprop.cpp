// Copyright Hugh Perkins 2014 hughperkins at gmail
//
// This Source Code Form is subject to the terms of the Mozilla Public License, 
// v. 2.0. If a copy of the MPL was not distributed with this file, You can 
// obtain one at http://mozilla.org/MPL/2.0/.

#include <iostream>
#include <stdexcept>

#include "OpenCLHelper.h"
#include "stringhelper.h"
#include "StatefulTimer.h"

#include "ActivationBackpropCpu.h"
#include "ActivationBackpropGpuNaive.h"

#include "ActivationBackprop.h"

using namespace std;

#undef VIRTUAL
#define VIRTUAL 
#undef STATIC
#define STATIC

STATIC ActivationBackprop *ActivationBackprop::instance( OpenCLHelper *cl, int numPlanes, int inputImageSize, ActivationFunction const *fn ) {
    return new ActivationBackpropGpuNaive( cl, numPlanes, inputImageSize, fn );
}
STATIC ActivationBackprop *ActivationBackprop::instanceForTest( OpenCLHelper *cl, int numPlanes, int inputImageSize, ActivationFunction const *fn) {
    return new ActivationBackpropGpuNaive( cl, numPlanes, inputImageSize, fn );
}
STATIC ActivationBackprop *ActivationBackprop::instanceSpecific( int idx, OpenCLHelper *cl, int numPlanes, int inputImageSize, ActivationFunction const *fn ) {
    if( idx == 0 ) {
        return new ActivationBackpropCpu( cl, numPlanes, inputImageSize, fn );
    }
    if( idx == 1 ) {
        return new ActivationBackpropGpuNaive( cl, numPlanes, inputImageSize, fn );
    }
    throw runtime_error("ActivationBackprop::instanceSpecific, idx not known: " + toString( idx ) );
}
ActivationBackprop::ActivationBackprop( OpenCLHelper *cl, int numPlanes, int inputImageSize, ActivationFunction const *fn ) :
        cl( cl ),
        numPlanes( numPlanes ),
        inputImageSize( inputImageSize ),
        fn( fn ),
        outputImageSize( inputImageSize ) {
}
VIRTUAL int ActivationBackprop::getInputSize( int batchSize ) {
    return batchSize * numPlanes * inputImageSize * inputImageSize;
}
VIRTUAL int ActivationBackprop::getOutputSize(int batchSize) {
    return batchSize * numPlanes * outputImageSize * outputImageSize;
}
VIRTUAL void ActivationBackprop::backpropErrors( int batchSize, float *inputs, float *errors, float *gradInput ) {
//    cout << "ActivationBackprop::backpropErrors( float * )" << endl;
    StatefulTimer::instance()->timeCheck("ActivationBackprop::backpropErrors float->wrapper start" );

    CLWrapper *inputsWrapper = cl->wrap( getInputSize(batchSize), inputs );
    CLWrapper *gradOutputWrapper = cl->wrap( getOutputSize(batchSize), errors );
    CLWrapper *gradInputWrapper = cl->wrap( getInputSize(batchSize), gradInput );

    inputsWrapper->copyToDevice();
    gradOutputWrapper->copyToDevice();

    backpropErrors( batchSize, inputsWrapper, gradOutputWrapper, gradInputWrapper );

    gradInputWrapper->copyToHost();

    delete inputsWrapper;
    delete gradOutputWrapper;
    delete gradInputWrapper;
    StatefulTimer::instance()->timeCheck("ActivationBackprop::backpropErrors float->wrapper end" );
}
VIRTUAL void ActivationBackprop::backpropErrors( int batchSize, CLWrapper *inputsWrapper, CLWrapper *gradOutputWrapper, CLWrapper *gradInputWrapper ) {
    throw runtime_error("ActivationBackprop::backpropErrors wrappers not implemented" );
}

