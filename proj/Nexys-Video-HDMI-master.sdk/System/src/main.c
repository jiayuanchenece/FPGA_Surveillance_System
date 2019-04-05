#include "main.h"

/*
 * XPAR redefines
 */
#define DYNCLK_BASEADDR XPAR_AXI_DYNCLK_0_BASEADDR
#define VGA_VDMA_ID XPAR_AXIVDMA_0_DEVICE_ID
#define DISP_VTC_ID XPAR_VTC_0_DEVICE_ID
#define VID_VTC_ID XPAR_VTC_1_DEVICE_ID
#define VID_GPIO_ID XPAR_AXI_GPIO_VIDEO_DEVICE_ID
#define VID_VTC_IRPT_ID XPAR_INTC_0_VTC_1_VEC_ID
#define VID_GPIO_IRPT_ID XPAR_INTC_0_GPIO_0_VEC_ID
#define SCU_TIMER_ID XPAR_AXI_TIMER_0_DEVICE_ID
#define UART_BASEADDR XPAR_UARTLITE_0_BASEADDR

#include "Platform/platform.h"
#include "Platform/platform_config.h"
#include "Platform/network_config.h"
#include "Platform/image_config.h"
#include "DisplayControl/display_ctrl.h"
#include "InterruptControl/intc.h"
#include "CameraControl/camera_ctrl.h"
#include "MotionDetection/motion_detection.h"
#include "logo.h"

// General Function Prototypes
void printMenu();

// Utilities Functions
void dexHex(unsigned int decimal, char* output, unsigned int n);

//Network Function prototypes
int network_init();
void print_ip(char *msg, ip_addr_t *ip);
void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw);
int send_image_to_server(u8 *frame, u32 width, u32 height, u32 stride);
int send_image(u8 *frame, u32 width, u32 height, u32 stride);
void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern);
int setup_client_conn();
void tcp_fasttmr(void);
void tcp_slowtmr(void);
void update_tcp_timer();

err_t dhcp_start(struct netif *netif);
void client_write(unsigned char* payload, int len);
void client_write_blocking(unsigned char *payload, int len);

void DemoInitialize();
void DemoRun();
void DemoPrintMenu();
void DemoChangeRes();
void DemoCRMenu();
void DemoInvertFrame(u8 *srcFrame, u8 *destFrame, u32 width, u32 height, u32 stride);
void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern);
void DemoScaleFrame(u8 *srcFrame, u8 *destFrame, u32 srcWidth, u32 srcHeight, u32 destWidth, u32 destHeight, u32 stride);
void DemoISR(void *callBackRef, void *pVideo);

//Function prototypes for callbacks
static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err);
static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
static void tcp_client_err(void *arg, err_t err);
static void tcp_client_close(struct tcp_pcb *pcb);

//Networking global variables
extern volatile int dhcp_timoutcntr;
extern volatile int TcpFastTmrFlag;
extern volatile int TcpSlowTmrFlag;
static struct netif server_netif;
struct netif *app_netif;
static struct tcp_pcb *c_pcb;
unsigned int server_data;
char is_connected;
int packet_sent, packet_rcvd = 0;

// Network Buffer
unsigned char buff[MAXBUFLEN];
unsigned char headerBuff[MAX_PACKET_LENGTH];
unsigned char outBuff[2*MAX_PACKET_LENGTH];

// Image Buffer
u8 frameBuf[DISPLAY_NUM_FRAMES][DEMO_MAX_FRAME /* +1 */];
u8 *pFrames[DISPLAY_NUM_FRAMES]; //array of pointers to the frame buffers



#define VGA
DisplayCtrl dispCtrl;
XAxiVdma vdma;
XAxiVdma overlaydma;
INTC intc;

int main() {

	init_platform();

	// Enable Mixer
	int * mixer_width = (int *) 0x50000010;  *mixer_width = 640;
	int * mixer_height = (int *) 0x50000018;  *mixer_height = 480;
	int * mixer_layer_enable =  (int *) 0x50000040;  *mixer_layer_enable = 0x8003;

	// Mixer Layer config
	int * mixer_layer_alpha = (int *) 0x50000200; *mixer_layer_alpha = 150;
	int * mixer_layer_startX = (int *) 0x50000208; *mixer_layer_startX = 0;
	int * mixer_layer_startY = (int *) 0x50000210; *mixer_layer_startY = 0;
	int * mixer_layer_width= (int *) 0x50000218; *mixer_layer_width = 640;
	int * mixer_layer_stride = (int *) 0x50000220; *mixer_layer_stride = 640*3;
	int * mixer_layer_height = (int *) 0x50000228; *mixer_layer_height = 480;
	int * mixer_layer_scale = (int *) 0x50000230; *mixer_layer_scale = 0;

	// Mixer Logo Config
	int * mixer_logo_startX = (int *) 0x50001000; *mixer_logo_startX = 0;
	int * mixer_logo_startY = (int *) 0x50001008; *mixer_logo_startY = 0;

	int * mixer_logo_width = (int *) 0x50001010; *mixer_logo_width = 64;
	int * mixer_logo_height = (int *) 0x50001018; *mixer_logo_height = 64;

	int * mixer_logo_scale = (int *) 0x50001020; *mixer_logo_scale = 0;
	int * mixer_logo_alpha = (int *) 0x50001028; *mixer_logo_alpha = 255;

	int * mixer_logo_red_buf = (int *) 0x50010000;
	for (int i = 0; i < 4096; i+=4) {
		*(mixer_logo_red_buf + i/4) = (Logo_R[i] << 24) | (Logo_R[i+1] << 16)  | (Logo_R[i+2] << 8) | (Logo_R[i+3]);
	}

	int * mixer_logo_green_buf = (int *) 0x50020000;
	for (int i = 0; i < 4096; i+=4) {
		*(mixer_logo_green_buf + i/4) = (Logo_G[i] << 24) | (Logo_G[i+1] << 16)  | (Logo_G[i+2] << 8) | (Logo_G[i+3]);
	}

	int * mixer_logo_blue_buf = (int *) 0x50030000;
	for (int i = 0; i < 4096;i+=4) {
		*(mixer_logo_blue_buf + i/4) = (Logo_B[i] << 24) | (Logo_B[i+1] << 16)  | (Logo_B[i+2] << 8) | (Logo_B[i+3]);
	}

	// Mixer start and auto start
	int * mixer_start = (int *) 0x50000000; *mixer_start = 0x00000081;

	// HDMI Output
	DemoInitialize();

	// Setup and enable camera
	camera_setup(pFrames, dispCtrl.curFrame);

	//Now enable interrupts
	platform_enable_interrupts();

	// Initialize Network Config
	 network_init();

	 xil_printf("Print Test Frame to RAM\n");
	 //DemoPrintTest(frameBuf[0], 640, 480, DEMO_STRIDE, 1);
	 //DemoPrintTest(frameBuf[1], 640, 480, DEMO_STRIDE, 0);

	// Parse User Input
	char userInput = 0;

	 // Flush UART FIFO
	 while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
	 	XUartLite_ReadReg(UART_BASEADDR, XUL_RX_FIFO_OFFSET);
	 }

	 // Print Option Menu
	 printMenu();

	 while (userInput != 'q') {

	 	// Wait for data on UART
	 	while (XUartLite_IsReceiveEmpty(UART_BASEADDR)) {}

	 	// Store the first character in the UART receive FIFO and echo it
	 	if (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
	 		userInput = XUartLite_ReadReg(UART_BASEADDR, XUL_RX_FIFO_OFFSET);
	 	} else {
	 		 //Refresh triggered by video detect interrupt
	 		userInput = 'r';
	 	}

	 	switch(userInput) {
	 		case '1':
	 			xil_printf("Make connection to server\n");
	 			setup_client_conn();
	 			printMenu();
	 			break;

	 		case '2':

	 			xil_printf("Make connection with the server\n");
	 			setup_client_conn();

				int task_complete = 0;
				while(1) {
					update_tcp_timer();
					if (is_connected && (task_complete == 0)) {

						for (int i = 0; i < 100; i++) {
							send_image_to_server(frameBuf[0], 640, 480, DEMO_STRIDE);
							//send_image_to_server(frameBuf[1], 640, 480, DEMO_STRIDE);
						}

						sprintf(outBuff,"EOV");
						client_write_blocking(outBuff, 3);

						task_complete = 1;
						break;

					}
				}

				tcp_client_close(c_pcb);

				printMenu();
	 			break;

	 		case '3':

	 			set_frame_a_address((u32)frameBuf[0]);
	 			set_frame_b_address((u32)frameBuf[0]);
	 			set_overlay_address((u32)frameBuf[1]);
	 			set_md_dma_address(XPAR_AXIDMA_0_BASEADDR);
	 			set_md_threshold(0x00010000);
	 			set_md_delay(0x00010000);

	 			start_md();

	 			xil_printf("Motion Detection Start: \n");
	 			xil_printf("frame_a_address %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_FRAME_A_OFFSET));
	 			xil_printf("frame_b_address %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_FRAME_B_OFFSET));
	 			xil_printf("overlay_address %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_OVERLAY_OFFSET));
	 			xil_printf("set_md_dma_address %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_DMA_OFFSET));
	 			xil_printf("set_threshold %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_THRESHOLD_OFFSET));
	 			xil_printf("md status  %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_STATUS_OFFSET));
	 			xil_printf("md delay %x\n", *(unsigned int *)(MOTION_BASE_ADDR + MD_DELAY_OFFSET));

	 			while (1) {
	 				if (get_md_result() & 0x2) {

//	 					xil_printf("MD Completed: Difference: %x\n", get_md_curr_diff());

	 					if (get_md_result() & 0x1) {
	 						xil_printf("Motion detected\n");
	 						xil_printf("MD Completed: Difference: %x\n", get_md_curr_diff());

	 						unsigned int location = (unsigned int *)(MOTION_BASE_ADDR + MD_LOCATION);
	 						unsigned int newX = 0x0000ffff & location;
	 						unsigned int newY = (0xffff0000 & location) >> 16;

//							*mixer_logo_startX = newX;
//							*mixer_logo_startY = newY;
//			 				unset_md();
//	 						break;
	 					}

//		 				xil_printf("Unset difference\n");
		 				unset_md();

//		 				xil_printf("Restart\n");
		 				start_md();
	 				}
	 			}

	 			printMenu();
	 			break;
	 		case 'q':
	 			if (is_connected) { tcp_client_close(c_pcb); }
	 			break;
	 		default:
	 			xil_printf("\n\nInvalid Selection\n\n");
	 			printMenu();
	 	}
	 }

	 cleanup_platform();
	 return 0;
}

void printMenu() {
	xil_printf("**************************************************\n\r");
	xil_printf("*             Network File Transfer              *\n\r");
	xil_printf("**************************************************\n\r");
	xil_printf("1 - Make Connection to Server\n\r");
	xil_printf("2 - Transfer Image\n\r");
	xil_printf("3 - Motion Detection\n\r");
	xil_printf("q - Quit\n\r");
	xil_printf("\n\r");
	xil_printf("\n\r");
	xil_printf("Enter a selection:");
}

void DemoInitialize() {
	int Status;
	XAxiVdma_Config *overlayVDMAConfig;
	XAxiVdma_Config *vdmaConfig;
	int i;

	/*
	 * Initialize an array of pointers to the 3 frame buffers
	 */
	for (i = 0; i < DISPLAY_NUM_FRAMES; i++) {
		pFrames[i] = frameBuf[i];
	}

	vdmaConfig = XAxiVdma_LookupConfig(VGA_VDMA_ID);
	if (!vdmaConfig) {
		xil_printf("No video DMA found for ID %d\r\n", VGA_VDMA_ID);
		return;
	}

	overlayVDMAConfig = XAxiVdma_LookupConfig(XPAR_AXIVDMA_2_DEVICE_ID);
	if (!overlayVDMAConfig) {
		xil_printf("No video DMA found for ID %d\r\n", XPAR_AXIVDMA_2_DEVICE_ID);
		return;
	}

	Status = XAxiVdma_CfgInitialize(&vdma, vdmaConfig, vdmaConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("VDMA Configuration Initialization failed %d\r\n", Status);
		return;
	}

	Status = XAxiVdma_CfgInitialize(&overlaydma, overlayVDMAConfig, overlayVDMAConfig->BaseAddress);
	if (Status != XST_SUCCESS) {
		xil_printf("Overlay VDMA Configuration Initialization failed %d\r\n", Status);
		return;
	}

	Status = DisplayInitialize(&dispCtrl, &vdma, &overlaydma,DISP_VTC_ID, DYNCLK_BASEADDR, pFrames, DEMO_STRIDE);
	if (Status != XST_SUCCESS) {
		xil_printf("Display Ctrl initialization failed during demo initialization%d\r\n", Status);
		return;
	}

	Status = DisplayStart(&dispCtrl);
	if (Status != XST_SUCCESS) {
		xil_printf("Couldn't start display during demo initialization%d\r\n", Status);
		return;
	}

	/*
	 * Initialize the Interrupt controller and start it.
	 */
//	Status = fnInitInterruptController(&intc);
//	if(Status != XST_SUCCESS) {
//		xil_printf("Error initializing interrupts");
//		return;
//	}
//	fnEnableInterrupts(&intc, &ivt[0], sizeof(ivt)/sizeof(ivt[0]));

	/*
	 * Set the Video Detect callback to trigger the menu to reset, displaying the new detected resolution
	 */
	 // VideoSetCallback(&videoCapt, DemoISR, &fRefresh);

	return;
}

void DemoChangeRes() {
	int fResSet = 0;
	int status;
	char userInput = 0;

	/* Flush UART FIFO */
	while (!XUartLite_IsReceiveEmpty(UART_BASEADDR)) {
			XUartLite_ReadReg(UART_BASEADDR, XUL_RX_FIFO_OFFSET);
		}

	while (!fResSet) {

		DemoCRMenu();

		/* Wait for data on UART */
		while (XUartLite_IsReceiveEmpty(UART_BASEADDR))
		{}

		/* Store the first character in the UART recieve FIFO and echo it */

		userInput = XUartLite_ReadReg(UART_BASEADDR, XUL_RX_FIFO_OFFSET);
		xil_printf("%c", userInput);
		status = XST_SUCCESS;
		switch (userInput) {
		case '1':
			status = DisplayStop(&dispCtrl);
			DisplaySetMode(&dispCtrl, &VMODE_640x480);
			DisplayStart(&dispCtrl);
			fResSet = 1;
			break;
		case '2':
			status = DisplayStop(&dispCtrl);
			DisplaySetMode(&dispCtrl, &VMODE_800x600);
			DisplayStart(&dispCtrl);
			fResSet = 1;
			break;
		case '3':
			status = DisplayStop(&dispCtrl);
			DisplaySetMode(&dispCtrl, &VMODE_1280x720);
			DisplayStart(&dispCtrl);
			fResSet = 1;
			break;
		case '4':
			status = DisplayStop(&dispCtrl);
			DisplaySetMode(&dispCtrl, &VMODE_1280x1024);
			DisplayStart(&dispCtrl);
			fResSet = 1;
			break;
		case '5':
			status = DisplayStop(&dispCtrl);
			DisplaySetMode(&dispCtrl, &VMODE_1920x1080);
			DisplayStart(&dispCtrl);
			fResSet = 1;
			break;
		case 'q':
			fResSet = 1;
			break;
		default :
			xil_printf("\n\rInvalid Selection");
		}
		if (status == XST_DMA_ERROR) {
			xil_printf("\n\rWARNING: AXI VDMA Error detected and cleared\n\r");
		}
	}
}

void DemoCRMenu() {
	xil_printf("\x1B[H"); //Set cursor to top left of terminal
	xil_printf("\x1B[2J"); //Clear terminal
	xil_printf("**************************************************\n\r");
	xil_printf("*             Nexys Video HDMI Demo              *\n\r");
	xil_printf("**************************************************\n\r");
	xil_printf("*Current Resolution: %28s*\n\r", dispCtrl.vMode.label);
	printf("*Pixel Clock Freq. (MHz): %23.3f*\n\r", dispCtrl.pxlFreq);
	xil_printf("**************************************************\n\r");
	xil_printf("\n\r");
	xil_printf("1 - %s\n\r", VMODE_640x480.label);
	xil_printf("2 - %s\n\r", VMODE_800x600.label);
	xil_printf("3 - %s\n\r", VMODE_1280x720.label);
	xil_printf("4 - %s\n\r", VMODE_1280x1024.label);
	xil_printf("5 - %s\n\r", VMODE_1920x1080.label);
	xil_printf("q - Quit (don't change resolution)\n\r");
	xil_printf("\n\r");
	xil_printf("Select a new resolution:");
}

void DemoInvertFrame(u8 *srcFrame, u8 *destFrame, u32 width, u32 height, u32 stride) {
	u32 xcoi, ycoi;
	u32 lineStart = 0;
	for(ycoi = 0; ycoi < height; ycoi++) {
		for(xcoi = 0; xcoi < (width * 3); xcoi+=3) {
			destFrame[xcoi + lineStart] = ~srcFrame[xcoi + lineStart];         //Red
			destFrame[xcoi + lineStart + 1] = ~srcFrame[xcoi + lineStart + 1]; //Blue
			destFrame[xcoi + lineStart + 2] = ~srcFrame[xcoi + lineStart + 2]; //Green
		}
		lineStart += stride;
	}
	/*
	 * Flush the framebuffer memory range to ensure changes are written to the
	 * actual memory, and therefore accessible by the VDMA.
	 */
	Xil_DCacheFlushRange((unsigned int) destFrame, DEMO_MAX_FRAME);
}

void DemoScaleFrame(u8 *srcFrame, u8 *destFrame, u32 srcWidth, u32 srcHeight, u32 destWidth, u32 destHeight, u32 stride) {
	float xInc, yInc; // Width/height of a destination frame pixel in the source frame coordinate system
	float xcoSrc, ycoSrc; // Location of the destination pixel being operated on in the source frame coordinate system
	float x1y1, x2y1, x1y2, x2y2; //Used to store the color data of the four nearest source pixels to the destination pixel
	int ix1y1, ix2y1, ix1y2, ix2y2; //indexes into the source frame for the four nearest source pixels to the destination pixel
	float xDist, yDist; //distances between destination pixel and x1y1 source pixels in source frame coordinate system

	int xcoDest, ycoDest; // Location of the destination pixel being operated on in the destination coordinate system
	int iy1; //Used to store the index of the first source pixel in the line with y1
	int iDest; //index of the pixel data in the destination frame being operated on

	int i;

	xInc = ((float) srcWidth - 1.0) / ((float) destWidth);
	yInc = ((float) srcHeight - 1.0) / ((float) destHeight);

	ycoSrc = 0.0;
	for (ycoDest = 0; ycoDest < destHeight; ycoDest++) {
		iy1 = ((int) ycoSrc) * stride;
		yDist = ycoSrc - ((float) ((int) ycoSrc));

		/*
		 * Save some cycles in the loop below by presetting the destination
		 * index to the first pixel in the current line
		 */
		iDest = ycoDest * stride;

		xcoSrc = 0.0;
		for (xcoDest = 0; xcoDest < destWidth; xcoDest++) {
			ix1y1 = iy1 + ((int) xcoSrc) * 3;
			ix2y1 = ix1y1 + 3;
			ix1y2 = ix1y1 + stride;
			ix2y2 = ix1y1 + stride + 3;

			xDist = xcoSrc - ((float) ((int) xcoSrc));

			/*
			 * For loop handles all three colors
			 */
			for (i = 0; i < 3; i++) {
				x1y1 = (float) srcFrame[ix1y1 + i];
				x2y1 = (float) srcFrame[ix2y1 + i];
				x1y2 = (float) srcFrame[ix1y2 + i];
				x2y2 = (float) srcFrame[ix2y2 + i];

				/*
				 * Bilinear interpolation function
				 */
				destFrame[iDest] = (u8) ((1.0-yDist)*((1.0-xDist)*x1y1+xDist*x2y1) + yDist*((1.0-xDist)*x1y2+xDist*x2y2));
				iDest++;
			}
			xcoSrc += xInc;
		}
		ycoSrc += yInc;
	}

	/*
	 * Flush the framebuffer memory range to ensure changes are written to the
	 * actual memory, and therefore accessible by the VDMA.
	 */
	Xil_DCacheFlushRange((unsigned int) destFrame, DEMO_MAX_FRAME);

	return;
}

void DemoISR(void *callBackRef, void *pVideo) {
	char *data = (char *) callBackRef;
	*data = 1; //set fRefresh to 1
}
/* ------------------------------------------------------------ */
/*							 FTP								*/
/* ------------------------------------------------------------ */
int send_image(u8 *frame, u32 width, u32 height, u32 stride) {


	int fileSize = width * height * 3;
	int headerSize = 4;
	int total_frag = fileSize / (MAX_PACKET_LENGTH - headerSize) + 1;

	for(int i = 0; i < height; i++) {
		client_write_blocking(frame + i * stride, 320*3);
		client_write_blocking(frame + i * stride + 320*3, 320*3);
	}

	xil_printf("Image Sent successfully");
	return 1;

}

int send_image_to_server(u8 *frame, u32 width, u32 height, u32 stride) {

	// Determine the number of packets needed
	int fileSize = width * height * 3;
	int headerSize = 4;
	int total_frag = fileSize / (MAX_PACKET_LENGTH - headerSize) + 1;
	//	xil_printf("Number of data packets required: %d\n", total_frag);

	//char tcpMessage[10];
	//char *command = "IMAGE";

//	client_write_blocking(command, strlen(command));
//	if (!is_connected) {
//		xil_printf("Disconnected from server\n\n");
//		return -1;
//	}

	int transferSize = 0;
	int frame_x = 0;
	int frame_y = 0;
	int frame_channel = 0;

	// File transfer start
	for (int frag_no = 0; frag_no < total_frag; frag_no++) {

		// Clear buffer
		// memset(headerBuff, '\0', MAX_PACKET_LENGTH);
		memset(outBuff, '\0', MAX_PACKET_LENGTH*2 );
		if (fileSize > MAX_PACKET_LENGTH-headerSize) transferSize = MAX_PACKET_LENGTH-headerSize;
		else transferSize = fileSize;

		sprintf(headerBuff,"%.3u",frag_no);
		//xil_printf("%s\n",headerBuff);

		// Out going buffer construction
		//int headerSize = strlen(headerBuff);

		memcpy(outBuff,headerBuff,headerSize);

		int escape_flag = 0;
		int pixelAddr = 0;
		int counter = 0;

		for (int y = frame_y; y < (height); y++) {
			pixelAddr = y * stride;
			for (int x = frame_x; x < width*3; x+=3) {
				for (int channel = frame_channel; channel < 3; channel++) {
					if (counter < transferSize){
						outBuff[headerSize + counter] = frame[pixelAddr + channel + x];
						counter++;
					} else {
						frame_x = x;
						frame_y = y;
						frame_channel = channel;
						escape_flag = 1;
						break;
					}
					frame_channel = 0;
				}
				if (escape_flag == 1) {
					 break;
				}
				frame_x = 0;
			}
			if (escape_flag == 1){
				 break;
			}
			frame_y = 0;
		}
		packet_sent = 0;
		// Message sent
		client_write(outBuff, headerSize + transferSize);

		xil_printf("frag_no : %d\n", frag_no);

		fileSize = fileSize - transferSize;
	}

	sprintf(outBuff,"EOF");
	//client_write(outBuff,3);
	xil_printf("Sending EOF\n");
	client_write_blocking(outBuff, 3);
	while(packet_rcvd == 0){


		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}
		//Process data queued after interupt
		xemacif_input(app_netif);
	}
	packet_rcvd = 0;

	xil_printf("Done send image");

	return 0;

}

/* ------------------------------------------------------------ */
/*							Network								*/
/* ------------------------------------------------------------ */
int network_init() {

	//Varibales for IP parameters
	ip_addr_t ipaddr, netmask, gw;

	//The mac address of the board. this should be unique per board
	unsigned char mac_ethernet_address[] = SRC_MAC_ADDR;

	//Network interface
	app_netif = &server_netif;

	//Defualt IP parameter values
	ipaddr.addr = 0;
	gw.addr = 0;
	netmask.addr = 0;

	//LWIP initialization
	lwip_init();

	//Setup Network interface and add to netif_list
	if (!xemac_add(app_netif, &ipaddr, &netmask,
						&gw, mac_ethernet_address,
						PLATFORM_EMAC_BASEADDR)) {
		xil_printf("Error adding N/W interface\n");
		return -1;
	}

	netif_set_default(app_netif);

	//Specify that the network is up
	netif_set_up(app_netif);

	dhcp_start(app_netif);
	dhcp_timoutcntr = 24;

	while(((app_netif->ip_addr.addr) == 0) && (dhcp_timoutcntr > 0)){
		//xil_printf("Bug here\n");
		xemacif_input(app_netif);
	}

	if (dhcp_timoutcntr <= 0) {
		if ((app_netif->ip_addr.addr) == 0) {
			xil_printf("DHCP Timeout\n");
			xil_printf("Configuring default IP of %s\n", SRC_IP4_ADDR);
			(void)inet_aton(SRC_IP4_ADDR, &(app_netif->ip_addr));
			(void)inet_aton(IP4_NETMASK, &(app_netif->netmask));
			(void)inet_aton(IP4_GATEWAY, &(app_netif->gw));
		}
	}

	ipaddr.addr = app_netif->ip_addr.addr;
	gw.addr = app_netif->gw.addr;
	netmask.addr = app_netif->netmask.addr;

	//Print connection settings
	print_ip_settings(&ipaddr, &netmask, &gw);

	return 0;

}

int setup_client_conn() {
	struct tcp_pcb *pcb;
	err_t err;
	ip_addr_t remote_addr;

	xil_printf("Setting up client connection\n");

	err = inet_aton(DEST_IP4_ADDR, &remote_addr);

	if (!err) {
		xil_printf("Invalid Server IP address: %d\n", err);
		return -1;
	}

	//Create new TCP PCB structure
	pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
	if (!pcb) {
		xil_printf("Error creating PCB. Out of Memory\n");
		return -1;
	}

	//Bind to specified @port
	err = tcp_bind(pcb, IP_ANY_TYPE, SRC_PORT);
	if (err != ERR_OK) {
		xil_printf("Unable to bind to port %d: err = %d\n", SRC_PORT, err);
		return -2;
	}

	//Connect to remote server (with callback on connection established)
	err = tcp_connect(pcb, &remote_addr, TCP_DEST_PORT, tcp_client_connected);
	if (err) {
		xil_printf("Error on tcp_connect: %d\n", err);
		tcp_client_close(pcb);
		return -1;
	}

	is_connected = 0;
	packet_sent = 0;

	xil_printf("Waiting for server to accept connection\n");

	return 0;
}

// UDP
void client_write(unsigned char* payload, int len) {

	ip_addr_t dest;
	inet_aton(DEST_IP4_ADDR, &dest);
	struct udp_pcb* my_pcb = udp_new();
	if(my_pcb == NULL) xil_printf("UDP setup failed\n");
	//xil_printf("udp_connect: %d\n",udp_connect(my_pcb,&dest, 9090));

	struct pbuf *p;
	p = pbuf_alloc(PBUF_TRANSPORT,len,PBUF_RAM);
	p->payload = payload;
	xemacif_input(app_netif);
	udp_sendto(my_pcb,p,&dest, DEST_PORT);
	pbuf_free(p);
	udp_remove(my_pcb);
}

//TCP
void client_write_blocking(unsigned char *payload, int len) {

	u8_t apiflags = TCP_WRITE_FLAG_COPY;
	packet_sent = 0;

	xil_printf("in client_write_blocking\n");

	while (tcp_sndbuf(c_pcb) < 5);

	xil_printf("in client_write_blocking 2\n");

	//Enqueue some data to send
	err_t err = tcp_write(c_pcb, payload, len, apiflags);
	if (err != ERR_OK) {
		xil_printf("TCP client: Error on tcp_write word value: %d\n", err);
		return ;
	}

	//send the data packet
	err = tcp_output(c_pcb);
	if (err != ERR_OK) {
		xil_printf("TCP client: Error on tcp_output WORD VALUE: %d\n",err);
		return ;
	}

	while(packet_sent == 0 && is_connected) {
		// Call tcp_tmr functions
		// Must be called regul	arly
		if (TcpFastTmrFlag) {
			tcp_fasttmr();
			TcpFastTmrFlag = 0;
		}
		if (TcpSlowTmrFlag) {
			tcp_slowtmr();
			TcpSlowTmrFlag = 0;
		}

		//Process data queued after interupt
		xemacif_input(app_netif);

		xil_printf("GG\n");
	}
	xil_printf("in client_write_blocking 3\n");
}

static void tcp_client_close(struct tcp_pcb *pcb) {
	err_t err;

	xil_printf("Closing Client Connection\n");

	if (pcb != NULL) {
		tcp_sent(pcb, NULL);
		tcp_recv(pcb,NULL);
		tcp_err(pcb, NULL);
		err = tcp_close(pcb);
		if (err != ERR_OK) {
			/* Free memory with abort */
			tcp_abort(pcb);
		}
	}
}

static err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err) {
	if (err != ERR_OK) {
		tcp_client_close(tpcb);
		xil_printf("Connection error\n");
		return err;
	}

	xil_printf("Connection to server established\n");

	//Store state (for callbacks)
	c_pcb = tpcb;
	is_connected = 1;

	//Set callback values & functions
	tcp_arg(c_pcb, NULL);
	tcp_recv(c_pcb, tcp_client_recv);
	tcp_sent(c_pcb, tcp_client_sent);
	tcp_err(c_pcb, tcp_client_err);

	//ADD CODE HERE to do when connection established

	return ERR_OK;
}

static err_t tcp_client_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
	//If no data, connection closed
	if (!p) {
		xil_printf("No data received\n");
		tcp_client_close(tpcb);
		return ERR_OK;
	}

//	//Print message
//	xil_printf("Packet received from Server, %d bytes\n", p->tot_len);
//
//	//Print packet contents to terminal
//	char* packet_data = (char*) malloc(p->tot_len);
//	pbuf_copy_partial(p, packet_data, p->tot_len, 0); //Note - inefficient way to access packet data
//
//	xil_printf("Packet received from Server, %s \n", packet_data);

	//Indicate done processing
	tcp_recved(tpcb, p->tot_len);

	//Free the received pbuf
	pbuf_free(p);
	packet_rcvd = 1;

	return 0;
}

static err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len){
	packet_sent = 1;
	xil_printf("Packet sent successfully, %d bytes\n", len);
	return 0;
}

static void tcp_client_err(void *arg, err_t err){
	LWIP_UNUSED_ARG(err);
	tcp_client_close(c_pcb);
	c_pcb = NULL;
	is_connected = 0;
	xil_printf("TCP connection aborted\n");
}

void update_tcp_timer() {
	if (TcpFastTmrFlag) {
		tcp_fasttmr();
		TcpFastTmrFlag = 0;
	}
	if (TcpSlowTmrFlag) {
		tcp_slowtmr();
		TcpSlowTmrFlag = 0;
	}

	//Process data queued after interupt
	xemacif_input(app_netif);
}

/* ------------------------------------------------------------ */
/*						   Utilities							*/
/* ------------------------------------------------------------ */
void DemoPrintTest(u8 *frame, u32 width, u32 height, u32 stride, int pattern) {
	u32 xcoi, ycoi;
	u32 iPixelAddr;
	u8 wRed, wBlue, wGreen;
	u32 wCurrentInt;
	double fRed, fBlue, fGreen, fColor;
	u32 xLeft, xMid, xRight, xInt;
	u32 yMid, yInt;
	double xInc, yInc;

	switch (pattern) {

	case 0:

		xInt = width / 4; //Four intervals, each with width/4 pixels
		xLeft = xInt * 3;
		xMid = xInt * 2 * 3;
		xRight = xInt * 3 * 3;
		xInc = 256.0 / ((double) xInt); //256 color intensities are cycled through per interval (overflow must be caught when color=256.0)

		yInt = height / 2; //Two intervals, each with width/2 lines
		yMid = yInt;
		yInc = 256.0 / ((double) yInt); //256 color intensities are cycled through per interval (overflow must be caught when color=256.0)

		fBlue = 0.0;
		fRed = 256.0;
		for(xcoi = 0; xcoi < (width*3); xcoi+=3) {
			/*
			 * Convert color intensities to integers < 256, and trim values >=256
			 */
			wRed = (fRed >= 256.0) ? 255 : ((u8) fRed);
			wBlue = (fBlue >= 256.0) ? 255 : ((u8) fBlue);
			iPixelAddr = xcoi;
			fGreen = 0.0;
			for(ycoi = 0; ycoi < height; ycoi++) {

				wGreen = (fGreen >= 256.0) ? 255 : ((u8) fGreen);
				frame[iPixelAddr] = wRed;
				frame[iPixelAddr + 1] = wBlue;
				frame[iPixelAddr + 2] = wGreen;
				if (ycoi < yMid) {
					fGreen += yInc;
				} else {
					fGreen -= yInc;
				}

				/*
				 * This pattern is printed one vertical line at a time, so the address must be incremented
				 * by the stride instead of just 1.
				 */
				iPixelAddr += stride;
			}

			if (xcoi < xLeft) {
				fBlue = 0.0;
				fRed -= xInc;
			} else if (xcoi < xMid) {
				fBlue += xInc;
				fRed += xInc;
			} else if (xcoi < xRight) {
				fBlue -= xInc;
				fRed -= xInc;
			} else {
				fBlue += xInc;
				fRed = 0;
			}
		}
		/*
		 * Flush the framebuffer memory range to ensure changes are written to the
		 * actual memory, and therefore accessible by the VDMA.
		 */
		Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
		break;
	case 1:

		xInt = width / 7; //Seven intervals, each with width/7 pixels
		xInc = 256.0 / ((double) xInt); //256 color intensities per interval. Notice that overflow is handled for this pattern.

		fColor = 0.0;
		wCurrentInt = 1;
		for(xcoi = 0; xcoi < (width*3); xcoi+=3) {

			/*
			 * Just draw white in the last partial interval (when width is not divisible by 7)
			 */
			if (wCurrentInt > 7) {
				wRed = 255;
				wBlue = 255;
				wGreen = 255;
			} else {
				if (wCurrentInt & 0b001)
					wRed = (u8) fColor;
				else
					wRed = 0;

				if (wCurrentInt & 0b010)
					wBlue = (u8) fColor;
				else
					wBlue = 0;

				if (wCurrentInt & 0b100)
					wGreen = (u8) fColor;
				else
					wGreen = 0;
			}

			iPixelAddr = xcoi;

			for(ycoi = 0; ycoi < height; ycoi++) {
				frame[iPixelAddr] = wRed;
				frame[iPixelAddr + 1] = wBlue;
				frame[iPixelAddr + 2] = wGreen;
				/*
				 * This pattern is printed one vertical line at a time, so the address must be incremented
				 * by the stride instead of just 1.
				 */
				iPixelAddr += stride;
			}

			fColor += xInc;
			if (fColor >= 256.0) {
				fColor = 0.0;
				wCurrentInt++;
			}
		}
		/*
		 * Flush the framebuffer memory range to ensure changes are written to the
		 * actual memory, and therefore accessible by the VDMA.
		 */
		Xil_DCacheFlushRange((unsigned int) frame, DEMO_MAX_FRAME);
		break;
	default :
		xil_printf("Error: invalid pattern passed to DemoPrintTest");
	}
}

void print_ip(char *msg, ip_addr_t *ip) {
	print(msg);
	xil_printf("%d.%d.%d.%d\n", ip4_addr1(ip), ip4_addr2(ip),
			ip4_addr3(ip), ip4_addr4(ip));
}

void print_ip_settings(ip_addr_t *ip, ip_addr_t *mask, ip_addr_t *gw) {
	print_ip("Board IP: ", ip);
	print_ip("Netmask : ", mask);
	print_ip("Gateway : ", gw);
}

void dexHex(unsigned int decimal, char* output, unsigned int n){
	unsigned int quotient = decimal;
	unsigned int remainder = 0;
	unsigned int i = 0;

	while (quotient != 0) {
		remainder = quotient % 16;
		if (remainder < 10 && i < n)
			output[i++] = 48 + remainder;
		else if (i < n)
			output[i++] = 55 + remainder;
		else
			break;
		quotient = quotient / 16;
	}
}


