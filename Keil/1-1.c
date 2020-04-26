#include <reg52.h>

sbit p34=P3^4;
sbit p35=P3^5;
sbit p36=P3^6;
sbit p37=P3^7;
sbit pe1=P2^4;

sbit p30=P3^0;
sbit p31=P3^1;
sbit p32=P3^2;
sbit p33=P3^3;
sbit pe2=P2^5;
		  
sbit p10=P1^0;
sbit p11=P1^1;
sbit p12=P1^2;
sbit p13=P1^3;
sbit pup=P2^6;

sbit p14=P1^4;
sbit p15=P1^5;
sbit p16=P1^6;
sbit p17=P1^7;
sbit pdown=P2^7;


//EleOne 表示电梯1现在处于的楼层，EleTwo表示现在电梯2处于的楼层
//EleOneState表示电梯1所处的状态，其中0静止，1向上，2向下
//EleTwoState表示电梯2所处的状态，数字的含义和EleOneState完全相同
int EleOne=1,EleTwo=1,EleOneState=0,EleTwoState=0;
int e1=0,e2=0,down=0,up=0;


void delay(int k){
	int i;
	for(i=0;i<k;i++){}
}


void display_LED(){

	int i=0;
	//依次点亮八个灯
	while(i<8){
	  P2=i;
	  pe1=(e1>>i)&1==1;//如果第i位为1，则对应的灯亮
	  pdown=(down>>i)&1==1;
	  pup=(up>>i)&1==1;
	  pe2=(e2>>i)&1==1;
	  i++;
	  delay(50);
	}

}

void display_Elevator(){

	P0=EleTwo+EleOne*16;
}

//捕获按键
void update_click(){
	//e1电梯按键
		if(p37){
			e1|=(1<<((p36*4)|(p35*2)|(p34)));
		}
	//down电梯按钮
		if(p17){
			down|=(1<<((p16*4)|(p15*2)|(p14)));
		}
	//up的按钮
		if(p13){
			up|=(1<<((p12*4)|(p11*2)|(p10)));
		}
	//e2按钮
		if(p33){
			e2|=(1<<((p32*4)|(p31*2)|(p30)));
		}
}
int downFloor(int *x,int *y){

    if((*x&(~((*x>>(*y-1))<<(*y-1))))>0){
        return 1;
    }else{
        return 0;
    }

}


void update_LED(){

	if(EleOneState==1){
		//处于上升状态
		up&=~(1<<(EleOne-1));
		e1&=~(1<<(EleOne-1));//第EleOne层置为0
		//是为了接到最高楼层的向下人员而处于的向上状态,而且当前楼层是向下人员中的最高层
		if((e1>>EleOne)<=0&&(up>>EleOne)<=0&&(down>>EleOne<=0)){
			down&=~(1<<(EleOne-1));
		}
	
	}else if(EleOneState==2){

		down&=~(1<<(EleOne-1));
		e1&=~(1<<(EleOne-1));
		//为了接到最低楼层的向上人员,而且当前楼层是向上人员中的最底层
		if(downFloor(&e1,&EleOne)<=0&&downFloor(&down,&EleOne)<=0&&downFloor(&up,&EleOne)<=0){
			up&=~(1<<(EleOne-1));
		}
	}


	if(EleTwoState==1){
        //处于上升状态
        up&=~(1<<(EleTwo-1));
        e2&=~(1<<(EleTwo-1));//第EleTwo层置为0
        //是为了接到最高楼层的向下人员而处于的向上状态,而且当前楼层是向下人员中的最高层
        if((e1>>EleTwo)<=0&&(up>>EleTwo)<=0&&(down>>EleTwo<=0)){
            down&=~(1<<(EleTwo-1));
        }

    }else if(EleTwoState==2){

        down&=~(1<<(EleTwo-1));
        e2&=~(1<<(EleTwo-1));
        //为了接到最低楼层的向上人员,而且当前楼层是向上人员中的最底层
        if(downFloor(&e1,&EleTwo)<=0&&downFloor(&down,&EleTwo)<=0&&downFloor(&up,&EleTwo)<=0){
            up&=~(1<<(EleTwo-1));
        }
    }
}


//Look电梯调度算法
void LookAlgorithm()
{
    //处理e1
    if(EleOneState==0){
        if((e1>>EleOne)>0){
            EleOneState=1;
            EleOne++;
        }else if(downFloor(&e1,&EleOne)){
            EleOneState=2;
            EleOne--;
        }

    }else if(EleOneState==1){
        //此时电梯处于向上工作状态，则只要电梯内部按下向上或者高楼层有人按下向上
        if((e1>>EleOne)>0||(up>>EleOne)>0){
            EleOne++;
        }else if((down>>EleOne)>0){
            //高楼层按下向下，则先上到最高楼层再向下
            EleOne++;
        }else if(downFloor(&e1,&EleOne)||downFloor(&up,&EleOne)||downFloor(&down,&EleOne)){
            //当电梯内部不需要向上而且高楼层无按钮按下，查看低楼层
            EleOneState=2;
            EleOne--;
        }else{
            EleOneState=0;
        }
    }else{
        if(downFloor(&e1,&EleOne)||downFloor(&down,&EleOne)){
            //处于向下状态时，如果电梯内部需要向下或者低楼层按下向下按钮
            EleOne--;
        }else if(downFloor(&up,&EleOne)){
            //低楼层有人向上，则降至最低楼层再向上
            EleOne--;
        }else if((e1>>EleOne)>0||(up>>EleOne)>0||(down>>EleOne)>0){
            //响应高楼层的请求
            EleOneState=1;
            EleOne++;
        }else{
            EleOneState=0;
        }

    }

    //处理e2
    if(EleTwoState==0){
        if((e2>>EleTwo)>0){
            EleTwoState=1;
            EleTwo++;
        }else if(downFloor(&e2,&EleTwo)){
            EleTwoState=2;
            EleTwo--;
        }

    }else if(EleTwoState==1){
        //此时电梯处于向上工作状态，则只要电梯内部按下向上或者高楼层有人按下向上
        if((e2>>EleTwo)>0||(up>>EleTwo)>0){
            EleTwo++;
        }else if((down>>EleTwo)>0){
            //高楼层按下向下，则先上到最高楼层再向下
            EleTwo++;
        }else if(downFloor(&e2,&EleTwo)||downFloor(&up,&EleTwo)||downFloor(&down,&EleTwo)){
            //当电梯内部不需要向上而且高楼层无按钮按下，查看低楼层
            EleTwoState=2;
            EleTwo--;
        }else{
            EleTwoState=0;
        }
    }else{
        if(downFloor(&e2,&EleTwo)||downFloor(&down,&EleTwo)){
            //处于向下状态时，如果电梯内部需要向下或者低楼层按下向下按钮
            EleTwo--;
        }else if(downFloor(&up,&EleTwo)){
            //低楼层有人向上，则降至最低楼层再向上
            EleTwo--;
        }else if((e2>>EleTwo)>0||(up>>EleTwo)>0||(down>>EleTwo)>0){
            //响应高楼层的请求
            EleTwoState=1;
            EleTwo++;
        }else{
            EleTwoState=0;
        }
    }

    if(EleOneState==0&&EleTwoState==0){
        //两个电梯都静止
        if(EleOne>EleTwo){
            //电梯1停止的楼层比电梯2高，则电梯1处理高楼层，电梯2处理低楼层
            if((up>>EleOne)>0||(down>>EleOne)>0){
                EleOneState=1;
                EleOne++;
            }

            if(downFloor(&up,&EleTwo)||downFloor(&down,&EleTwo)){
                EleTwoState=2;
                EleTwo--;
            }
            if(EleOneState==0&&EleTwoState==0){
                if(up!=0||down!=0){
                    //请求在楼层中间
                    EleOne--;
                    EleOneState=2;
                    EleTwo++;
                    EleTwoState=1;
                }
            }
        }else{

            if((up>>EleTwo)>0||(down>>EleTwo)>0){
                EleTwoState=1;
                EleTwo++;
            }

            if(downFloor(&up,&EleOne)||downFloor(&down,&EleOne)){
                EleOneState=2;
                EleOne--;
            }

            if(EleOneState==0&&EleTwoState==0){
                if(up!=0||down!=0){
                    //请求在楼层中间
                    EleOne++;
                    EleOneState=1;
                    EleTwo--;
                    EleTwoState=2;
                }
            }
        }
    }else if(EleOneState==0){
            //电梯1静止
            if(EleTwoState==1){
                //电梯二在向上
                if(downFloor(&up,&EleOne)){
                    //而且电梯2下面也有请求,则电梯2处理低楼层请求
                    EleOne--;
                    EleOneState=2;
                }
            }else{
                if((down>>EleOne)>0){
                    EleOne++;
                    EleOneState=1;
                }
            }

    }else if(EleTwoState==0){

            if(EleOneState==1){
                if(downFloor(&up,&EleTwo)){
                    EleTwo--;
                    EleTwoState=2;
                }
            }else{
                if((down>>EleTwo)>0){
                    EleTwo++;
                    EleTwoState=1;
                }
            }
        }

        update_LED();

}


void main()
{
	int i;
 	while(1){
		i=0;
		display_LED();
		display_Elevator();
		update_click();
		LookAlgorithm();
		display_Elevator();
		
		while(i<120){
			update_click();
			display_LED();
			i++;
		}
	}
}