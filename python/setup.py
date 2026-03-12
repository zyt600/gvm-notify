import os
from pybind11.setup_helpers import Pybind11Extension, build_ext
from setuptools import setup

cuda_path = os.environ.get("CUDA_PATH", "/usr/local/cuda")
src = os.path.join(os.path.dirname(__file__), os.pardir)

ext = Pybind11Extension(
    "gvm_notify",
    sources=[
        "bindings.cpp",
        os.path.join(src, "gvm_notify.cpp"),
        os.path.join(src, "uvm_utils.cpp"),
    ],
    include_dirs=[src, cuda_path + "/include"],
    library_dirs=[cuda_path + "/lib64"],
    libraries=["cuda", "pthread"],
    cxx_std=17,
)

setup(
    name="gvm-notify",
    version="0.1.0",
    description="GVM GPU notification listener with Python callback support",
    ext_modules=[ext],
    cmdclass={"build_ext": build_ext},
    python_requires=">=3.7",
)
