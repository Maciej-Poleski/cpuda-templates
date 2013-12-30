/*
 * Copyright 2013  Maciej Poleski <maciej.poleski@uj.edu.pl>
 */

#ifndef MODULE_H
#define MODULE_H

#include <string>

struct data3d {
    int x, y, z;
};

struct gridDim_t : public data3d {};

struct blockDim_t : public data3d {};

struct blockIdx_t : public data3d {};

struct threadIdx_t : public data3d {};

class Module;

class Function
{
public:
    /**
     * @param module Module in which we look for this function
     * @param name Name of kernel function (must be marked with __global__)
     */
    Function(const Module& module, const std::string& name);

    /**
     * @param args Kernel parameters as provided in cuLaunchKernel
     */
    void run(void* args[],
             gridDim_t gridDim,
             blockDim_t blockDim,
             blockIdx_t blockIdx,
             threadIdx_t threadIdx);

private:
    const void* functionHandle;
};


class Module
{
    friend Function::Function(const Module& module, const std::string& name);
public:
    /**
     * @param name Name of ptx file (including extension ex. myKernl.ptx)
     */
    Module(const std::string& name);

    /**
     * Initialize global module state. Must be invoked exactly one time before
     * kernel
     * @param gridDim gridDim as provided to kernel
     */
    void initializeModule(gridDim_t gridDim);

    /**
     * Releases any resources associated witch this module. Must be invoked
     * exactly one time after last thread exit.
     */
    void cleanupModule();

    /**
     * Initialize shared block state. Must be invoked exactly one time before
     * start of any thread in given block. Must be invoked after
     * initializeModule
     * @param gridDim gridDim as provided to kernel
     * @param blockIdx blockIdx of block which is about to launch
     */
    void initializeBlock(gridDim_t gridDim, blockIdx_t blockIdx);

    /**
     * Releases any resources associated with given block. Must be invoked
     * exactly one time after last thread in given block finished but before
     * cleanupModule()
     * @param gridDim gridDim as provided to kernel
     * @param blockIdx blockIdx of block which is about to finish
     */
    void releaseBlock(gridDim_t gridDim, blockIdx_t blockIdx);

private:
    const void* moduleHandle;
};


#endif // MODULE_H
