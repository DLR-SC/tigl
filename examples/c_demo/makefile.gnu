# This is a makefile to build the example TiGL program on Linux or Mac

# Please modify the variables TIGL_HOME and TIXI_HOME
# to the appropriate install locations
TIGL_HOME=<tigl install folder>
TIXI_HOME=<tixi install folder>

c_demo: c_demo.c
	@echo "Building c_demo"
	@gcc -o c_demo c_demo.c -I$(TIGL_HOME)/include -I$(TIXI_HOME)/include \
	-L$(TIGL_HOME)/lib -L$(TIXI_HOME)/lib -L$(TIGL_HOME)/lib64 -L$(TIXI_HOME)/lib64 -lTIGL -lTIXI
