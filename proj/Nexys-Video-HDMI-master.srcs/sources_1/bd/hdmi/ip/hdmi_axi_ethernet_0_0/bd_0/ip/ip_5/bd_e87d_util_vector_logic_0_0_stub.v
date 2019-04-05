// Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
// --------------------------------------------------------------------------------
// Tool Version: Vivado v.2018.2 (win64) Build 2258646 Thu Jun 14 20:03:12 MDT 2018
// Date        : Thu Feb 21 01:10:53 2019
// Host        : DESKTOP-GD2UNUH running 64-bit major release  (build 9200)
// Command     : write_verilog -force -mode synth_stub -rename_top bd_e87d_util_vector_logic_0_0 -prefix
//               bd_e87d_util_vector_logic_0_0_ bd_e87d_util_vector_logic_0_0_stub.v
// Design      : bd_e87d_util_vector_logic_0_0
// Purpose     : Stub declaration of top-level module interface
// Device      : xc7a200tsbg484-1
// --------------------------------------------------------------------------------

// This empty module with port declaration file causes synthesis tools to infer a black box for IP.
// The synthesis directives are for Synopsys Synplify support to prevent IO buffer insertion.
// Please paste the declaration into a Verilog source file or add the file as an additional source.
(* X_CORE_INFO = "util_vector_logic_v2_0_1_util_vector_logic,Vivado 2018.2" *)
module bd_e87d_util_vector_logic_0_0(Op1, Res)
/* synthesis syn_black_box black_box_pad_pin="Op1[0:0],Res[0:0]" */;
  input [0:0]Op1;
  output [0:0]Res;
endmodule