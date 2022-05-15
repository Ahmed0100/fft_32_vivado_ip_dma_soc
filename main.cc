
#include "xparameters.h"
#include "xaxidma.h"
#include "xio.h"
#include "complex.h"
#include "xcounter.h"
#define N 8

const int rev8[N]={0,4,2,6,1,5,3,7};
const float complex W[N/2]={1-0*I, 0.7071067811865476-0.7071067811865475*I,0.0-1*I,-0.7071067811865475-0.7071067811865476*I};

void bitReverse(float complex dataIn[N],float complex dataOut[N])
{
	for(int i=0;i<N;i++)
	{
		dataOut[i]=dataIn[rev8[i]];
	}
}
void fftStages(float complex dataIn[N],float complex dataOut[N])
{
	float complex temp1[N],temp2[N];
	stage1:for(int i=0;i<N;i=i+2)
	{
		temp1[i]=dataIn[i]+dataIn[i+1];
		temp1[i+1]=dataIn[i]-dataIn[i+1]
	}
	stage2:for(int i=0;i<N;i=i+4)
	{
		for(int j=0;j<2;j++)
		{
			temp2[i+j]=temp1[i+j]+W[2*j] * dataIn[i+j+2];
			temp2[i+2+j]=temp1[i+j]-W[2*j]*dataIn[i+j+2];
		}
	}
	stage3:for(int i=0;i<N/2;i=i++)
	{

		dataOut[i]=temp2[i]+W[i] * temp2[i+4];
		dataOut[i+4]=temp2[i]-W[i] * temp2[i+4];
	}

}
int main()
{
	xil_printf("START\n");
	const float complex fft_input_sw[N]={11+23*I,32+10*I,91+94*I,15+69*I,47+96*I,44+12*I,96+17*I,49+58*I};
	float complex fft_output_sw[N];
	float complex fft_input_rev_sw[N];

	XCounter xc;
	XCounter* xcptr = &xc;
	XCounter_Initialize(xcptr, 0);
	XCounter_EnableAutoRestart(xcptr);
	XCounter_Start(xcptr);

	int t1 = XCounter_Get_return(xcptr);


	bitReverse(fft_input_sw,fft_input_rev_sw);
	fftStages(fft_input_rev_sw,fft_output_sw);
	int t2 = XCounter_Get_return(xcptr);


	const float fft_input[2*N] = {0x41300000,0x41b80000,0x42000000,0x41200000,0x42b60000,0x42bc0000,0x41700000,0x428a0000,
		0x423c0000,0x42c00000,0x42300000,0x41400000,0x42c00000,0x41880000,0x42440000,0x42680000};

	int status;
	int init_addr = XPAR_BRAM_0_BASEADDR;
	for(int i=0;i<2*N;i++)
	{
		XIo_Out32(init_addr,fft_input[i]);
		init_addr += 4;
	}
	XAxiDma my_dma;
	XAxiDma_Config * cnfg;
	cnfg = XAxiDma_LookupConfigBaseAddr(XPAR_AXI_DMA_0_BASEADDR);
	XAxiDma_CfgInitialize(&my_dma,cnfg);

	int t3 = XCounter_Get_return(xcptr);
	status = XAxiDma_SimpleTransfer(&my_dma,XPAR_BRAM_0_BASEADDR+sizeof(float)*2*N,sizeof(float)*2*N,XAXIDMA_DEVICE_TO_DMA);
	if(status != XST_SUCCESS)
	{
		;//xil_printf("WRITING DATA FAILED\n");
	}
	status = XAxiDma_SimpleTransfer(&my_dma,XPAR_BRAM_0_BASEADDR,sizeof(float)*2*N,XAXIDMA_DMA_TO_DEVICE);
	if(status != XST_SUCCESS)
	{
		;//xil_printf("READING DATA FAILED\n");
	}
	while ((XAxiDma_Busy(&my_dma,XAXIDMA_DEVICE_TO_DMA)) ||
		(XAxiDma_Busy(&my_dma,XAXIDMA_DMA_TO_DEVICE))) {
			;//xil_printf("w.\n");
	}
	int t4 = XCounter_Get_return(xcptr);

	xil_printf("SW FFT cycles: %d\r\n", t2-t1);
	xil_printf("HW FFT cycles: %d\r\n", t4-t3);


	int out_addr = XPAR_BRAM_0_BASEADDR + 2*N*sizeof(float);
	xil_printf("transfer done\n");
	for(int i=0;i<2*N;i++)
	{
		xil_printf("Out %d = %d",i,XIo_In32(out_addr));
		out_addr += 4;
	}

}
