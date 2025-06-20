#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <easyx/easyx.h>

#define ROW 10 // 定义行列的常量
#define COL 10
#define MineNum 10 // 定义地雷的数量

#define WIN_ANIM_FRAMES 18 //呆菇
#define WIN_ANIM_FRAMES2 44 // 巫师，假设每个胜利动画有44帧
IMAGE winAnim1[WIN_ANIM_FRAMES];
IMAGE winAnim2[WIN_ANIM_FRAMES2];

// 定义图形界面的宽度和高度
IMAGE img[14];//IMAGE图像对象
void loadResource()
{
	for (int i = 0; i < 12; i++)
	{
		char imgPath[100] = {0};
		sprintf(imgPath, "picture/%d.png", i);//将格式化的数据写入字符串imgPath
		loadimage(&img[i], imgPath);
		//loadimage这个函数用于从文件中读取图像
	}
	
	// 加载胜利动画帧
	for (int i = 0; i < WIN_ANIM_FRAMES; i++) {
		char path1[100];//呆菇
		sprintf(path1, "picture/菇菇的认可_%d.png", i); // win1_0.png, win1_1.png...
		loadimage(&winAnim1[i], path1, 120, 120); // 缩放为120x120
	}
	
	for (int i = 0; i < WIN_ANIM_FRAMES2; i++) {
		char path2[100];//巫师
		sprintf(path2, "picture/跳舞的巫师%d.png", i); // win2_0.png, win2_1.png...// 缩放为120x120
		loadimage(&winAnim2[i], path2, 120, 120);
	}
	
	// 额外加载两张失败图片
	loadimage(&img[12], "picture/盯人的鸟子.jpg",120,120);
	loadimage(&img[13], "picture/红脸蛋子的鸟子.jpg",120,120);
}

// 函数声明
void show(int map[][COL]);//在控制台打印map
void init(int map[][COL]);
void draw(int map[][COL]);
void mouseMsg(ExMessage *msg, int map[][COL]);
void boomblank(int map[][COL], int row, int col);
int gameover(int map[][COL], int row, int col);
bool showWinAnimation();

int main()
{
	// 初始化窗口
	initgraph(400, 400, EX_SHOWCONSOLE);
	// 扫雷地图
	int map[ROW][COL] = {0}; // 初始化地图
	init(map);
	show(map);
	
	// 游戏主循环  
	while (1)
	{
		// 处理用户输入
		ExMessage msg;//ExMessage这个结构体用于保存鼠标消息
		while (peekmessage(&msg, EX_MOUSE))//peekmessage这个函数用于获取一个消息，并立即返回
		{//如果获取到了消息，返回 true；如果当前没有消息，返回 false
			switch (msg.message)//message消息标识
			{
				case WM_LBUTTONDOWN: // 鼠标左键按下和右键按下
			case WM_RBUTTONDOWN:
				mouseMsg(&msg, map);
				int ret = gameover(map,msg.y/40, msg.x/40);
				if (ret == -1)
				{
					draw(map); // 先绘制当前地图
					
					// 绘制半透明黑色背景（模拟弹窗）
					setfillcolor(RGB(200, 200, 200));
					setbkcolor(RGB(200, 200, 200));
					settextcolor(BLACK);
					fillrectangle(50, 50, 350, 350);
					
					// 显示失败文字
					settextstyle(24, 0, "微软雅黑");
					outtextxy(150, 60, "游戏失败！");
					
					// 显示两张失败图片
					putimage(70, 120, &img[12]);  // lose1.jpg
					putimage(210, 120, &img[13]); // lose2.jpg
					
					// 显示按钮文字
					outtextxy(90, 280, "再来一次(Y)");
					outtextxy(240, 280, "退出(N)");
					
					// 等待玩家按键选择
					while (1)
					{
						if (GetAsyncKeyState('Y') & 0x8000) // 按 Y 键重新开始
						{
							init(map);
							break;
						}
						else if (GetAsyncKeyState('N') & 0x8000) // 按 N 键退出
						{
							exit(0);
						}
						Sleep(100); // 避免 CPU 占用过高
					}
				}
				else if(ret == 1)
				{
					draw(map);
					if (showWinAnimation()) {
						init(map);
					} else {
						exit(0);
					}
				}
				system("cls");
				// system 函数来自 <stdlib.h> 头文件，它能够将一个字符串参数作为系统命令传递给
				// 操作系统的命令解释器来执行。system("cls"); 中的 "cls" 是 Windows 系统下的命令
				//	，其功能是清空命令提示符窗口（控制台窗口）中的所有内容，使窗口变为空白。
				printf("judege:%d\n", ret);
				show(map);
				break;
			}
		}
		
		draw(map);
	}
	
//	show(map);
	
	getchar();//让程序暂停，等待用户输入一个字符后才会继续执行后续代码或者退出程序
	return 0;
}

//在控制台打印map
void show(int map[][COL])
{
	int i = 0;
	int k = 0;
	for (i = 0; i < ROW; i++)
	{
		for (k = 0; k < COL; k++)
		{
			printf("%3d", map[i][k]);
		}
		printf("\n");
	}
}

// 初始化数据
void init(int map[][COL])
{
	loadResource();//调用函数
	
	srand((unsigned int)time(NULL));//初始化随机数生成器种子
	//time(NULL) 是 <time.h> 头文件里的一个函数，它会返回当前的系统时间
	// 随机设置十个雷 用-1表示
	
	//把map初始化为0
	memset(map,0,sizeof(int)*ROW*COL);
	//memset将一块指定内存区域的每个字节都设置为特定的值
	int i = 0;
	for (i = 0; i < MineNum;)
	{
		// 数组的有效下表 0-9
		//rand() 函数生成一个介于0到RAND_MAX之间的伪随机整数。RAND_MAX 是一个常量，通常至少为 32767。
		int r = rand() % ROW;
		int c = rand() % COL;
		// 判断是否已经有雷
		if (map[r][c] == 0)
		{
			map[r][c] = -1;
			i++;
		}
	}
	
	// 计算周围雷的数量，把雷周围九宫格的数字加1
	for (int i = 0; i < ROW; i++)
	{
		for (int k = 0; k < COL; k++)
		{
			// 找到雷，并遍历雷所在的九宫格
			if (map[i][k] == -1)
			{
				for (int r = i - 1; r <= i + 1; r++)
				{
					for (int c = k - 1; c <= k + 1; c++)
					{
						// 判断是否越界
						if (r >= 0 && r < ROW && c >= 0 && c < COL)
						{
							// 对周围的数据加1
							if (map[r][c] != -1)
							{
								map[r][c]++;
							}
						}
					}
				}
			}
		}
	}
	
	// 加密格子，上面盖盖子
	for (int i = 0; i < ROW; i++)
	{
		for (int k = 0; k < COL; k++)
		{
			map[i][k] += 20;
		}
	}
}

// 绘制
void draw(int map[][COL])
{
	// 贴图，根据map里面的数字，贴图
	for (int i = 0; i < ROW; i++)
	{
		for (int k = 0; k < COL; k++)
		{
			if (map[i][k] >= 0 && map[i][k] <= 8) // 0-8
			{//putimage用于在当前设备上绘制指定图像
				putimage(40 * k, 40 * i, &img[map[i][k]]);
			}
			else if (map[i][k] == -1) // 地雷
			{
				putimage(40 * k, 40 * i, &img[9]);
			}
			else if (map[i][k] >= 19 && map[i][k] <= 28) // 加密格子
			{
				putimage(40 * k, 40 * i, &img[10]);
			}
			else if (map[i][k] >= 39) // 标记格子
			{
				putimage(40 * k, 40 * i, &img[11]);
			}
		}
	}
}

// 鼠标操作数据
void mouseMsg(ExMessage *msg, int map[][COL])
{
	// 先根据鼠标的坐标，计算出点击的是哪个格子
	int r = msg->y / 40;//ExMessage结构体中的short x; 鼠标的 x 坐标
	int c = msg->x / 40;//short y;鼠标的 y 坐标
	// 左键打开格子，右键标记格子
	if (msg->message == WM_LBUTTONDOWN)//左键按下
	{
		// 打开格子
		if (map[r][c] >= 19 && map[r][c] <= 28)
		{
			map[r][c] -= 20;
			boomblank(map, r, c);
			//boomblank函数，检测一下是不是空白格子
		}
	}
	// 右键标记格子
	else if (msg->message == WM_RBUTTONDOWN)//右键按下
	{
		// 是否能够标记：如果没有打开，就可以标记
		if (map[r][c] >= 19 && map[r][c] <= 28)
		{//改变二维数组map点的数据来改变贴图
			map[r][c] += 20;
		}
		else if (map[r][c] >= 39)
		{
			map[r][c] -= 20;
		}
	}
}

// 点击空白格子，打开周围的格子
void boomblank(int map[][COL], int row, int col)
{
	//判断是不是空白格子
	if (map[row][col] == 0)
	{ 
		for(int r =row-1;r <=row+1; r++)
		{
			for(int c =col-1;c <=col+1; c++)
			{
				if(r >= 0 && r < ROW && c >= 0 && c < COL)  //没越界
				{
					if(map[r][c] >= 19 && map[r][c] <= 28)  //没打开
					{
						map[r][c] -= 20;
						boomblank(map, r, c);  //递归
					}
				}
			}
		}
	}
}
//游戏结束条件  输了返回-1，没结束返回0，赢了返回1
int gameover(int map[][COL],int row,int col)
{
	//点到雷结束，输
	if(map[row][col] == -1 || map[row][col] == 19)
	{
		return -1;
	}
	
	//点到格子结束，赢
	int cnt =0;
	for(int i =0;i < ROW;i++)
	{
		for(int k =0;k < COL;k++)
		{
			//统计打开的格子的数量
			if(map[i][k] >= 0 && map[i][k] <= 8)
			{
				cnt++;
			}
		}
	}
	if(ROW*COL - MineNum == cnt)
	{
		return 1;
	}
	return 0;
}

// 显示胜利动画的函数
bool showWinAnimation()
{
	DWORD startTime = GetTickCount();
	int frame1 = 0; // 呆菇的帧
	int frame2 = 0; // 巫师的帧
	
	// 绘制半透明背景
	setfillcolor(BLACK);
	setbkcolor(BLACK);
	solidrectangle(50, 50, 350, 350);
	
	// 显示文字
	settextcolor(RGB(250, 250, 250));
	settextstyle(24, 0, "微软雅黑");
	outtextxy(170, 60, "胜利啦！");
	
	// 动画循环
	while (1) {
		// 分别计算两个动画的当前帧
		frame1 = (GetTickCount() - startTime) / 100 % WIN_ANIM_FRAMES;  // 呆菇：17帧
		frame2 = (GetTickCount() - startTime) / 40 % WIN_ANIM_FRAMES2; // 巫师：44帧
		
		// 绘制动画帧
		putimage(70, 120, &winAnim1[frame1]);  // 呆菇
		putimage(210, 120, &winAnim2[frame2]); // 巫师
		
		// 显示按钮提示
		settextcolor(RGB(250, 250, 250));
		outtextxy(80, 280, "按 Y 再来一局");
		outtextxy(240, 280, "按 N 退出");
		
		// 检查按键
		if (GetAsyncKeyState('Y') & 0x8000) return true;
		if (GetAsyncKeyState('N') & 0x8000) return false;
		
		FlushBatchDraw();
	}
}

