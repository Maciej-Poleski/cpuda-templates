#define __device__
#define __shared__
#define __global__


/* Część stała */
namespace detail
{
struct data3d {
    int x, y, z;
};

struct gridDim_t : public data3d {};

struct blockDim_t : public data3d {};

struct blockIdx_t : public data3d {};

struct threadIdx_t : public data3d {};
}

static thread_local detail::gridDim_t gridDim;
static thread_local detail::blockDim_t blockDim;
static thread_local detail::blockIdx_t blockIdx;
static thread_local detail::threadIdx_t threadIdx;

namespace detail
{
static int grid_flat_size(gridDim_t gridDim)
{
    return gridDim.x * gridDim.y * gridDim.z;
}
static int block_flat_idx(gridDim_t gridDim, blockIdx_t blockIdx)
{
    return gridDim.x * gridDim.y * blockIdx.z +
           gridDim.x * blockIdx.y +
           blockIdx.x;
}
static int block_flat_idx()
{
    return gridDim.x * gridDim.y * blockIdx.z +
           gridDim.x * blockIdx.y +
           blockIdx.x;
}
}

#include <type_traits>

/* Część zmienna (dostosowana do pliku .cu) */
namespace detail
{
namespace shared
{
struct {
    int tab[123], temp, *ptr, arr[2];
}** fun1;
struct {
    char tab[12], a, b, *c;
    int q, w[2], *e;
}** fun2;
}
};


/* Tutaj rozpoczyna się przerobiony blok kodu z pliku .cu */
extern "C"
{

    __device__ int fun1(int* argIntStar, float argFloat)
    {
        //__shared__ int tab[123], temp, *ptr, arr[2];
        auto& tab = detail::shared::fun1[detail::block_flat_idx()]->tab;
        auto& temp = detail::shared::fun1[detail::block_flat_idx()]->temp;
        auto& ptr = detail::shared::fun1[detail::block_flat_idx()]->ptr;
        auto& arr = detail::shared::fun1[detail::block_flat_idx()]->arr;
        ptr = &tab[5];
        tab[5] = argIntStar[threadIdx.x];
        argIntStar[6] = *ptr;
        return 10;
    }

    __global__ void fun2(char* argCharStar, int argInt)
    {
        //__shared__ char tab[12], a, b, *c;
        auto& tab = detail::shared::fun2[detail::block_flat_idx()]->tab;
        auto& a = detail::shared::fun2[detail::block_flat_idx()]->a;
        auto& b = detail::shared::fun2[detail::block_flat_idx()]->b;
        auto& c = detail::shared::fun2[detail::block_flat_idx()]->c;
        //__shared__ int q, w[2], *e;
        auto& q = detail::shared::fun2[detail::block_flat_idx()]->q;
        auto& w = detail::shared::fun2[detail::block_flat_idx()]->w;
        auto& e = detail::shared::fun2[detail::block_flat_idx()]->e;
    }

}
/* Tutaj kończy się przerobiony blok kodu z pliku .cu */

/* Implementacja zmiennej funkcjonalności */

// Ta funkcja musi zostać uruchomiona przed uruchomieniem kernela
extern "C" void _kernel_global_init(detail::gridDim_t gridDim)
{
    using fun1_t = std::remove_pointer<decltype(detail::shared::fun1)>::type;
    detail::shared::fun1 = new fun1_t[detail::grid_flat_size(gridDim)];
    using fun2_t = std::remove_pointer<decltype(detail::shared::fun2)>::type;
    detail::shared::fun2 = new fun2_t[detail::grid_flat_size(gridDim)];
}

// Ta funkcja musi zostać uruchomiona po zakończeniu pracy kernela
extern "C" void _kernel_global_deinit()
{
    delete [] detail::shared::fun1;
    delete [] detail::shared::fun2;
}

// Ta funkcja musi zostać uruchomiona przed odpaleniem pierwszego wątku
// należącego do danego bloku
extern "C" void _kernel_block_init(detail::gridDim_t gridDim, detail::blockIdx_t blockIdx)
{
    using fun1_t = std::remove_pointer<decltype(detail::shared::fun1)>::type;
    detail::shared::fun1[detail::block_flat_idx(gridDim, blockIdx)] = new std::remove_pointer<fun1_t>::type;
    using fun2_t = std::remove_pointer<decltype(detail::shared::fun2)>::type;
    detail::shared::fun2[detail::block_flat_idx(gridDim, blockIdx)] = new std::remove_pointer<fun2_t>::type;
}

// Ta funkcja musi zostać uruchomiona po zakończeniu ostatniegą wątku
// należącego do danego bloku, ale przed uruchomieniem funkcji _kernel_global_deinit
extern "C" void _kernel_block_deinit(detail::gridDim_t gridDim, detail::blockIdx_t blockIdx)
{
    delete detail::shared::fun1[detail::block_flat_idx(gridDim, blockIdx)];
    delete detail::shared::fun2[detail::block_flat_idx(gridDim, blockIdx)];
}

/* Realizacja ABI */
extern "C" void fun2_start(void* args[],
                           detail::gridDim_t gridDim,
                           detail::blockDim_t blockDim,
                           detail::blockIdx_t blockIdx,
                           detail::threadIdx_t threadIdx)
{
    ::gridDim = gridDim;
    ::blockDim = blockDim;
    ::blockIdx = blockIdx;
    ::threadIdx = threadIdx;
    fun2(
        *reinterpret_cast<char**>(args[0]),
        *reinterpret_cast<int*>(args[1])
    );  // To może się jeszcze uda poprawić
}
