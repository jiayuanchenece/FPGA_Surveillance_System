-- Copyright 1986-2018 Xilinx, Inc. All Rights Reserved.
-- --------------------------------------------------------------------------------
-- Tool Version: Vivado v.2018.2 (win64) Build 2258646 Thu Jun 14 20:03:12 MDT 2018
-- Date        : Tue Mar 26 15:26:08 2019
-- Host        : DESKTOP-GD2UNUH running 64-bit major release  (build 9200)
-- Command     : write_vhdl -force -mode synth_stub
--               C:/Users/Tristan/Documents/Nexys-Video-HDMI-master/proj/Nexys-Video-HDMI-master.srcs/sources_1/bd/hdmi/ip/hdmi_v_mix_0_0/hdmi_v_mix_0_0_stub.vhdl
-- Design      : hdmi_v_mix_0_0
-- Purpose     : Stub declaration of top-level module interface
-- Device      : xc7a200tsbg484-1
-- --------------------------------------------------------------------------------
library IEEE;
use IEEE.STD_LOGIC_1164.ALL;

entity hdmi_v_mix_0_0 is
  Port ( 
    s_axi_CTRL_AWADDR : in STD_LOGIC_VECTOR ( 17 downto 0 );
    s_axi_CTRL_AWVALID : in STD_LOGIC;
    s_axi_CTRL_AWREADY : out STD_LOGIC;
    s_axi_CTRL_WDATA : in STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_CTRL_WSTRB : in STD_LOGIC_VECTOR ( 3 downto 0 );
    s_axi_CTRL_WVALID : in STD_LOGIC;
    s_axi_CTRL_WREADY : out STD_LOGIC;
    s_axi_CTRL_BRESP : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_CTRL_BVALID : out STD_LOGIC;
    s_axi_CTRL_BREADY : in STD_LOGIC;
    s_axi_CTRL_ARADDR : in STD_LOGIC_VECTOR ( 17 downto 0 );
    s_axi_CTRL_ARVALID : in STD_LOGIC;
    s_axi_CTRL_ARREADY : out STD_LOGIC;
    s_axi_CTRL_RDATA : out STD_LOGIC_VECTOR ( 31 downto 0 );
    s_axi_CTRL_RRESP : out STD_LOGIC_VECTOR ( 1 downto 0 );
    s_axi_CTRL_RVALID : out STD_LOGIC;
    s_axi_CTRL_RREADY : in STD_LOGIC;
    ap_clk : in STD_LOGIC;
    ap_rst_n : in STD_LOGIC;
    interrupt : out STD_LOGIC;
    s_axis_video_TVALID : in STD_LOGIC;
    s_axis_video_TREADY : out STD_LOGIC;
    s_axis_video_TDATA : in STD_LOGIC_VECTOR ( 23 downto 0 );
    s_axis_video_TKEEP : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axis_video_TSTRB : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axis_video_TUSER : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video_TLAST : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video_TID : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video_TDEST : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video1_TVALID : in STD_LOGIC;
    s_axis_video1_TREADY : out STD_LOGIC;
    s_axis_video1_TDATA : in STD_LOGIC_VECTOR ( 23 downto 0 );
    s_axis_video1_TKEEP : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axis_video1_TSTRB : in STD_LOGIC_VECTOR ( 2 downto 0 );
    s_axis_video1_TUSER : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video1_TLAST : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video1_TID : in STD_LOGIC_VECTOR ( 0 to 0 );
    s_axis_video1_TDEST : in STD_LOGIC_VECTOR ( 0 to 0 );
    m_axis_video_TVALID : out STD_LOGIC;
    m_axis_video_TREADY : in STD_LOGIC;
    m_axis_video_TDATA : out STD_LOGIC_VECTOR ( 23 downto 0 );
    m_axis_video_TKEEP : out STD_LOGIC_VECTOR ( 2 downto 0 );
    m_axis_video_TSTRB : out STD_LOGIC_VECTOR ( 2 downto 0 );
    m_axis_video_TUSER : out STD_LOGIC_VECTOR ( 0 to 0 );
    m_axis_video_TLAST : out STD_LOGIC_VECTOR ( 0 to 0 );
    m_axis_video_TID : out STD_LOGIC_VECTOR ( 0 to 0 );
    m_axis_video_TDEST : out STD_LOGIC_VECTOR ( 0 to 0 )
  );

end hdmi_v_mix_0_0;

architecture stub of hdmi_v_mix_0_0 is
attribute syn_black_box : boolean;
attribute black_box_pad_pin : string;
attribute syn_black_box of stub : architecture is true;
attribute black_box_pad_pin of stub : architecture is "s_axi_CTRL_AWADDR[17:0],s_axi_CTRL_AWVALID,s_axi_CTRL_AWREADY,s_axi_CTRL_WDATA[31:0],s_axi_CTRL_WSTRB[3:0],s_axi_CTRL_WVALID,s_axi_CTRL_WREADY,s_axi_CTRL_BRESP[1:0],s_axi_CTRL_BVALID,s_axi_CTRL_BREADY,s_axi_CTRL_ARADDR[17:0],s_axi_CTRL_ARVALID,s_axi_CTRL_ARREADY,s_axi_CTRL_RDATA[31:0],s_axi_CTRL_RRESP[1:0],s_axi_CTRL_RVALID,s_axi_CTRL_RREADY,ap_clk,ap_rst_n,interrupt,s_axis_video_TVALID,s_axis_video_TREADY,s_axis_video_TDATA[23:0],s_axis_video_TKEEP[2:0],s_axis_video_TSTRB[2:0],s_axis_video_TUSER[0:0],s_axis_video_TLAST[0:0],s_axis_video_TID[0:0],s_axis_video_TDEST[0:0],s_axis_video1_TVALID,s_axis_video1_TREADY,s_axis_video1_TDATA[23:0],s_axis_video1_TKEEP[2:0],s_axis_video1_TSTRB[2:0],s_axis_video1_TUSER[0:0],s_axis_video1_TLAST[0:0],s_axis_video1_TID[0:0],s_axis_video1_TDEST[0:0],m_axis_video_TVALID,m_axis_video_TREADY,m_axis_video_TDATA[23:0],m_axis_video_TKEEP[2:0],m_axis_video_TSTRB[2:0],m_axis_video_TUSER[0:0],m_axis_video_TLAST[0:0],m_axis_video_TID[0:0],m_axis_video_TDEST[0:0]";
attribute X_CORE_INFO : string;
attribute X_CORE_INFO of stub : architecture is "hdmi_v_mix_0_0_v_mix,Vivado 2018.2";
begin
end;
