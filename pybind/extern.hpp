/*
 * Copyright (c) 2020 NVIDIA Corporation.
 * Copyright (c) 2018-2020 Chris Choy (chrischoy@ai.stanford.edu).
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please cite "4D Spatio-Temporal ConvNets: Minkowski Convolutional Neural
 * Networks", CVPR'19 (https://arxiv.org/abs/1904.08755) if you use any part
 * of the code.
 */
#include "coordinate_map.hpp"
#include "coordinate_map_cpu.hpp"
#include "coordinate_map_key.hpp"
#include "coordinate_map_manager.hpp"
#include "errors.hpp"
#include "types.hpp"
#include "utils.hpp"

#ifndef CPU_ONLY
#include "allocators.cuh"
#include "coordinate_map_gpu.cuh"
#endif

#include <torch/extension.h>

#include <pybind11/numpy.h>
#include <pybind11/operators.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace minkowski {

/*************************************
 * Convolution
 *************************************/
template <typename coordinate_type>
at::Tensor
ConvolutionForwardCPU(at::Tensor const &in_feat,                         //
                      at::Tensor const &kernel,                          //
                      default_types::stride_type const &kernel_size,     //
                      default_types::stride_type const &kernel_stride,   //
                      default_types::stride_type const &kernel_dilation, //
                      RegionType::Type const region_type,                //
                      at::Tensor const &offset,                          //
                      CoordinateMapKey *p_in_map_key,                    //
                      CoordinateMapKey *p_out_map_key,                   //
                      cpu_manager_type<coordinate_type> *p_map_manager);

template <typename coordinate_type>
std::pair<at::Tensor, at::Tensor>
ConvolutionBackwardCPU(at::Tensor const &in_feat,                         //
                       at::Tensor &grad_out_feat,                         //
                       at::Tensor const &kernel,                          //
                       default_types::stride_type const &kernel_size,     //
                       default_types::stride_type const &kernel_stride,   //
                       default_types::stride_type const &kernel_dilation, //
                       RegionType::Type const region_type,                //
                       at::Tensor const &offsets,                         //
                       CoordinateMapKey *p_in_map_key,                    //
                       CoordinateMapKey *p_out_map_key,                   //
                       cpu_manager_type<coordinate_type> *p_map_manager);

#ifndef CPU_ONLY
template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
at::Tensor ConvolutionForwardGPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);

template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
std::pair<at::Tensor, at::Tensor> ConvolutionBackwardGPU(
    at::Tensor const &in_feat,                         //
    at::Tensor &grad_out_feat,                         //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);
#endif

/*************************************
 * Convolution Transpose
 *************************************/
template <typename coordinate_type>
at::Tensor ConvolutionTransposeForwardCPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    bool generate_new_coordinates,                     //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    cpu_manager_type<coordinate_type> *p_map_manager);

template <typename coordinate_type>
std::pair<at::Tensor, at::Tensor> ConvolutionTransposeBackwardCPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &grad_out_feat,                   //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offsets,                         //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    cpu_manager_type<coordinate_type> *p_map_manager);

#ifndef CPU_ONLY
template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
at::Tensor ConvolutionTransposeForwardGPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    bool generate_new_coordinates,                     //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);

template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
std::pair<at::Tensor, at::Tensor> ConvolutionTransposeBackwardGPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &grad_out_feat,                   //
    at::Tensor const &kernel,                          //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);
#endif

/*************************************
 * Local Pooling
 *************************************/
template <typename coordinate_type>
std::pair<at::Tensor, at::Tensor>
LocalPoolingForwardCPU(at::Tensor const &in_feat,
                       default_types::stride_type const &kernel_size,     //
                       default_types::stride_type const &kernel_stride,   //
                       default_types::stride_type const &kernel_dilation, //
                       RegionType::Type const region_type,                //
                       at::Tensor const &offset,                          //
                       PoolingMode::Type pooling_mode,                    //
                       CoordinateMapKey *p_in_map_key,                    //
                       CoordinateMapKey *p_out_map_key,                   //
                       cpu_manager_type<coordinate_type> *p_map_manager);

template <typename coordinate_type>
at::Tensor
LocalPoolingBackwardCPU(at::Tensor const &in_feat,                         //
                        at::Tensor const &grad_out_feat,                   //
                        at::Tensor const &num_nonzero,                     //
                        default_types::stride_type const &kernel_size,     //
                        default_types::stride_type const &kernel_stride,   //
                        default_types::stride_type const &kernel_dilation, //
                        RegionType::Type const region_type,                //
                        at::Tensor const &offset,                          //
                        PoolingMode::Type pooling_mode,                    //
                        CoordinateMapKey *p_in_map_key,                    //
                        CoordinateMapKey *p_out_map_key,                   //
                        cpu_manager_type<coordinate_type> *p_map_manager);

#ifndef CPU_ONLY
template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
std::pair<at::Tensor, at::Tensor> LocalPoolingForwardGPU(
    at::Tensor const &in_feat,
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    PoolingMode::Type pooling_mode,                    //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);

template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
at::Tensor LocalPoolingBackwardGPU(
    at::Tensor const &in_feat,                         //
    at::Tensor const &grad_out_feat,                   //
    at::Tensor const &num_nonzero,                     //
    default_types::stride_type const &kernel_size,     //
    default_types::stride_type const &kernel_stride,   //
    default_types::stride_type const &kernel_dilation, //
    RegionType::Type const region_type,                //
    at::Tensor const &offset,                          //
    PoolingMode::Type pooling_mode,                    //
    CoordinateMapKey *p_in_map_key,                    //
    CoordinateMapKey *p_out_map_key,                   //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);
#endif

/*************************************
 * Global Pooling
 *************************************/
template <typename coordinate_type>
std::tuple<at::Tensor, at::Tensor>
GlobalPoolingForwardCPU(at::Tensor const &in_feat,
                        PoolingMode::Type const pooling_mode, //
                        CoordinateMapKey *p_in_map_key,       //
                        CoordinateMapKey *p_out_map_key,      //
                        cpu_manager_type<coordinate_type> *p_map_manager);

template <typename coordinate_type>
at::Tensor
GlobalPoolingBackwardCPU(at::Tensor const &in_feat, at::Tensor &grad_out_feat,
                         at::Tensor const &num_nonzero,
                         PoolingMode::Type const pooling_mode, //
                         CoordinateMapKey *p_in_map_key,       //
                         CoordinateMapKey *p_out_map_key,      //
                         cpu_manager_type<coordinate_type> *p_map_manager);

#ifndef CPU_ONLY
template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
std::tuple<at::Tensor, at::Tensor> GlobalPoolingForwardGPU(
    at::Tensor const &in_feat,
    PoolingMode::Type const pooling_mode, //
    CoordinateMapKey *p_in_map_key,       //
    CoordinateMapKey *p_out_map_key,      //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);

template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
at::Tensor GlobalPoolingBackwardGPU(
    at::Tensor const &in_feat,            //
    at::Tensor &grad_out_feat,            //
    at::Tensor const &num_nonzero,        //
    PoolingMode::Type const pooling_mode, //
    CoordinateMapKey *p_in_map_key,       //
    CoordinateMapKey *p_out_map_key,      //
    gpu_manager_type<coordinate_type, TemplatedAllocator> *p_map_manager);
#endif

/*************************************
 * Quantization
 *************************************/
std::vector<py::array> quantize_np(
    py::array_t<int32_t, py::array::c_style | py::array::forcecast> coords);

std::vector<at::Tensor> quantize_th(at::Tensor &coords);

/*
vector<py::array> quantize_label_np(
    py::array_t<int, py::array::c_style | py::array::forcecast> coords,
    py::array_t<int, py::array::c_style | py::array::forcecast> labels,
    int invalid_label);


vector<at::Tensor> quantize_label_th(at::Tensor coords, at::Tensor labels,
                                     int invalid_label);

at::Tensor quantization_average_features(at::Tensor in_feat, at::Tensor in_map,
                                         at::Tensor out_map, int out_nrows,
                                         int mode);
*/

#ifndef CPU_ONLY
template <typename th_int_type>
torch::Tensor coo_spmm(torch::Tensor const &rows, torch::Tensor const &cols,
                       torch::Tensor const &vals, int64_t const dim_i,
                       int64_t const dim_j, torch::Tensor const &mat2,
                       int64_t spmm_algorithm_id);

std::pair<size_t, size_t> get_memory_info();
#endif

} // end namespace minkowski

namespace py = pybind11;

template <typename coordinate_type>
void instantiate_cpu_func(py::module &m, const std::string &dtypestr) {
  m.def((std::string("ConvolutionForwardCPU") + dtypestr).c_str(),
        &minkowski::ConvolutionForwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("ConvolutionBackwardCPU") + dtypestr).c_str(),
        &minkowski::ConvolutionBackwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("ConvolutionTransposeForwardCPU") + dtypestr).c_str(),
        &minkowski::ConvolutionTransposeForwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());
  m.def((std::string("ConvolutionTransposeBackwardCPU") + dtypestr).c_str(),
        &minkowski::ConvolutionTransposeBackwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("LocalPoolingForwardCPU") + dtypestr).c_str(),
        &minkowski::LocalPoolingForwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());
  m.def((std::string("LocalPoolingBackwardCPU") + dtypestr).c_str(),
        &minkowski::LocalPoolingBackwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("GlobalPoolingForwardCPU") + dtypestr).c_str(),
        &minkowski::GlobalPoolingForwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());
  m.def((std::string("GlobalPoolingBackwardCPU") + dtypestr).c_str(),
        &minkowski::GlobalPoolingBackwardCPU<coordinate_type>,
        py::call_guard<py::gil_scoped_release>());

  /*
    m.def((std::string("BroadcastForwardCPU") + dtypestr).c_str(),
          &mink::BroadcastForwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("BroadcastBackwardCPU") + dtypestr).c_str(),
          &mink::BroadcastBackwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #ifndef CPU_ONLY
    m.def((std::string("BroadcastForwardGPU") + dtypestr).c_str(),
          &mink::BroadcastForwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("BroadcastBackwardGPU") + dtypestr).c_str(),
          &mink::BroadcastBackwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #endif

    m.def((std::string("PruningForwardCPU") + dtypestr).c_str(),
          &mink::PruningForwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("PruningBackwardCPU") + dtypestr).c_str(),
          &mink::PruningBackwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #ifndef CPU_ONLY
    m.def((std::string("PruningForwardGPU") + dtypestr).c_str(),
          &mink::PruningForwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("PruningBackwardGPU") + dtypestr).c_str(),
          &mink::PruningBackwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #endif

    m.def((std::string("UnionForwardCPU") + dtypestr).c_str(),
          &mink::UnionForwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("UnionBackwardCPU") + dtypestr).c_str(),
          &mink::UnionBackwardCPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #ifndef CPU_ONLY
    m.def((std::string("UnionForwardGPU") + dtypestr).c_str(),
          &mink::UnionForwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
    m.def((std::string("UnionBackwardGPU") + dtypestr).c_str(),
          &mink::UnionBackwardGPU<MapType, Dtype>,
          py::call_guard<py::gil_scoped_release>());
  #endif
  */
}

#ifndef CPU_ONLY
template <typename coordinate_type,
          template <typename C> class TemplatedAllocator>
void instantiate_gpu_func(py::module &m, const std::string &dtypestr) {
  m.def((std::string("ConvolutionForwardGPU") + dtypestr).c_str(),
        &minkowski::ConvolutionForwardGPU<coordinate_type, TemplatedAllocator>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("ConvolutionBackwardGPU") + dtypestr).c_str(),
        &minkowski::ConvolutionBackwardGPU<coordinate_type, TemplatedAllocator>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("ConvolutionTransposeForwardGPU") + dtypestr).c_str(),
        &minkowski::ConvolutionTransposeForwardGPU<coordinate_type,
                                                   TemplatedAllocator>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("ConvolutionTransposeBackwardGPU") + dtypestr).c_str(),
        &minkowski::ConvolutionTransposeBackwardGPU<coordinate_type,
                                                    TemplatedAllocator>,
        py::call_guard<py::gil_scoped_release>());

  m.def((std::string("LocalPoolingForwardGPU") + dtypestr).c_str(),
        &minkowski::LocalPoolingForwardGPU<coordinate_type, TemplatedAllocator>,
        py::call_guard<py::gil_scoped_release>());
  m.def(
      (std::string("LocalPoolingBackwardGPU") + dtypestr).c_str(),
      &minkowski::LocalPoolingBackwardGPU<coordinate_type, TemplatedAllocator>,
      py::call_guard<py::gil_scoped_release>());

  m.def(
      (std::string("GlobalPoolingForwardGPU") + dtypestr).c_str(),
      &minkowski::GlobalPoolingForwardGPU<coordinate_type, TemplatedAllocator>,
      py::call_guard<py::gil_scoped_release>());
  m.def(
      (std::string("GlobalPoolingBackwardGPU") + dtypestr).c_str(),
      &minkowski::GlobalPoolingBackwardGPU<coordinate_type, TemplatedAllocator>,
      py::call_guard<py::gil_scoped_release>());
}
#endif

void non_templated_cpu_func(py::module &m) {
  m.def("quantize_np", &minkowski::quantize_np);
  m.def("quantize_th", &minkowski::quantize_th);
}

#ifndef CPU_ONLY
void non_templated_gpu_func(py::module &m) {
  m.def("coo_spmm_int32", &minkowski::coo_spmm<int32_t>,
        py::call_guard<py::gil_scoped_release>());
  m.def("coo_spmm_int64", &minkowski::coo_spmm<int64_t>,
        py::call_guard<py::gil_scoped_release>());
}
#endif

void initialize_non_templated_classes(py::module &m) {
  // Enums
  py::enum_<minkowski::GPUMemoryAllocatorBackend::Type>(
      m, "GPUMemoryAllocatorType")
      .value("PYTORCH", minkowski::GPUMemoryAllocatorBackend::Type::PYTORCH)
      .value("CUDA", minkowski::GPUMemoryAllocatorBackend::Type::CUDA)
      .export_values();

  py::enum_<minkowski::CUDAKernelMapMode::Mode>(m, "CUDAKernelMapMode")
      .value("MEMORY_EFFICIENT",
             minkowski::CUDAKernelMapMode::Mode::MEMORY_EFFICIENT)
      .value("SPEED_OPTIMIZED",
             minkowski::CUDAKernelMapMode::Mode::SPEED_OPTIMIZED)
      .export_values();

  py::enum_<minkowski::MinkowskiAlgorithm::Mode>(m, "MinkowskiAlgorithm")
      .value("DEFAULT", minkowski::MinkowskiAlgorithm::Mode::DEFAULT)
      .value("MEMORY_EFFICIENT",
             minkowski::MinkowskiAlgorithm::Mode::MEMORY_EFFICIENT)
      .value("SPEED_OPTIMIZED",
             minkowski::MinkowskiAlgorithm::Mode::SPEED_OPTIMIZED)
      .export_values();

  py::enum_<minkowski::CoordinateMapBackend::Type>(m, "CoordinateMapType")
      .value("CPU", minkowski::CoordinateMapBackend::Type::CPU)
      .value("CUDA", minkowski::CoordinateMapBackend::Type::CUDA)
      .export_values();

  py::enum_<minkowski::RegionType::Type>(m, "RegionType")
      .value("HYPER_CUBE", minkowski::RegionType::Type::HYPER_CUBE)
      .value("HYPER_CROSS", minkowski::RegionType::Type::HYPER_CROSS)
      .value("CUSTOM", minkowski::RegionType::Type::CUSTOM)
      .export_values();

  py::enum_<minkowski::PoolingMode::Type>(m, "PoolingMode")
      .value("LOCAL_SUM_POOLING",
             minkowski::PoolingMode::Type::LOCAL_SUM_POOLING)
      .value("LOCAL_AVG_POOLING",
             minkowski::PoolingMode::Type::LOCAL_AVG_POOLING)
      .value("LOCAL_MAX_POOLING",
             minkowski::PoolingMode::Type::LOCAL_MAX_POOLING)
      .value("GLOBAL_SUM_POOLING_DEFAULT",
             minkowski::PoolingMode::Type::GLOBAL_SUM_POOLING_DEFAULT)
      .value("GLOBAL_AVG_POOLING_DEFAULT",
             minkowski::PoolingMode::Type::GLOBAL_AVG_POOLING_DEFAULT)
      .value("GLOBAL_MAX_POOLING_DEFAULT",
             minkowski::PoolingMode::Type::GLOBAL_MAX_POOLING_DEFAULT)
      .value("GLOBAL_SUM_POOLING_KERNEL",
             minkowski::PoolingMode::Type::GLOBAL_SUM_POOLING_KERNEL)
      .value("GLOBAL_AVG_POOLING_KERNEL",
             minkowski::PoolingMode::Type::GLOBAL_AVG_POOLING_KERNEL)
      .value("GLOBAL_MAX_POOLING_KERNEL",
             minkowski::PoolingMode::Type::GLOBAL_MAX_POOLING_KERNEL)
      .value("GLOBAL_SUM_POOLING_PYTORCH_INDEX",
             minkowski::PoolingMode::Type::GLOBAL_SUM_POOLING_PYTORCH_INDEX)
      .value("GLOBAL_AVG_POOLING_PYTORCH_INDEX",
             minkowski::PoolingMode::Type::GLOBAL_AVG_POOLING_PYTORCH_INDEX)
      .value("GLOBAL_MAX_POOLING_PYTORCH_INDEX",
             minkowski::PoolingMode::Type::GLOBAL_MAX_POOLING_PYTORCH_INDEX)
      .export_values();

  // Classes
  py::class_<minkowski::CoordinateMapKey>(m, "CoordinateMapKey")
      .def(py::init<minkowski::default_types::size_type>())
      .def(py::init<minkowski::default_types::stride_type, std::string>())
      .def("__repr__", &minkowski::CoordinateMapKey::to_string)
      .def("is_key_set", &minkowski::CoordinateMapKey::is_key_set)
      .def("get_coordinate_size",
           &minkowski::CoordinateMapKey::get_coordinate_size)
      .def("get_key", &minkowski::CoordinateMapKey::get_key)
      .def("set_key", (void (minkowski::CoordinateMapKey::*)(
                          minkowski::default_types::stride_type, std::string)) &
                          minkowski::CoordinateMapKey::set_key)
      .def("get_tensor_stride", &minkowski::CoordinateMapKey::get_tensor_stride)
      .def(py::self == py::self);
}

template <typename manager_type>
void instantiate_manager(py::module &m, const std::string &dtypestr) {
  py::class_<manager_type>(
      m, (std::string("CoordinateMapManager") + dtypestr).c_str())
      .def(py::init<>())
      .def(py::init<minkowski::MinkowskiAlgorithm::Mode,
                    minkowski::default_types::size_type>())
      .def("__repr__",
           py::overload_cast<>(&manager_type::to_string, py::const_))
      .def("print_coordinate_map",
           py::overload_cast<minkowski::CoordinateMapKey const *>(
               &manager_type::to_string, py::const_))
      .def("insert_and_map", &manager_type::insert_and_map)
      .def("stride", &manager_type::py_stride)
      .def("origin", &manager_type::py_origin)
      .def("get_coordinates", &manager_type::get_coordinates)
      .def("get_coordinate_map_keys", &manager_type::get_coordinate_map_keys)
      .def("size", py::overload_cast<minkowski::CoordinateMapKey const *>(
                       &manager_type::size, py::const_))
      .def("kernel_map", &manager_type::kernel_map)
      .def("origin_map", &manager_type::origin_map_th)
      .def("origin_map_size", &manager_type::origin_map_size);
}

bool is_cuda_available() {
#ifndef CPU_ONLY
  return true;
#else
  return false;
#endif
}

int cuda_version() {
#if defined(CUDART_VERSION)
  return CUDART_VERSION;
#else
  return -1;
#endif
}

std::pair<size_t, size_t> get_gpu_memory_info() {
#ifndef CPU_ONLY
  return minkowski::get_memory_info();
#else
  return std::make_pair(0, 0);
#endif
}
