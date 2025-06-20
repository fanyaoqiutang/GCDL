#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <easyx/easyx.h>

// 游戏配置
typedef struct {
	int rows;
	int cols;
	int mines;//地雷数量
} GameConfig;

// 难度级别
typedef enum {
	EASY,
	MEDIUM,
	HARD
} Difficulty;//枚举定义

// 不同难度配置
GameConfig difficultyConfig[] = {
	{10, 10, 10},   // 简单
	{16, 16, 40},   // 中等
	{16, 30, 99}    // 困难
};

// 当前游戏配置
GameConfig currentConfig;

#define WIN_ANIM_FRAMES 18 //呆菇
#define WIN_ANIM_FRAMES2 44 // 巫师，假设每个胜利动画有44帧
IMAGE winAnim1[WIN_ANIM_FRAMES];
IMAGE winAnim2[WIN_ANIM_FRAMES2];
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
void show(int** map);//在控制台打印map
void init(int** map);
void draw(int** map, int remainingMines);
void mouseMsg(ExMessage *msg, int** map, int* remainingMines);
void boomblank(int** map, int row, int col);
int gameover(int** map, int row, int col);
bool showWinAnimation();
Difficulty showDifficultyMenu();
void drawMineCounter(int remainingMines);

int main()
{
	// 隐藏控制台窗口（仅适用于 GUI 程序）
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_HIDE); // SW_HIDE 隐藏，SW_SHOW 显示
	
	
	// 返回主页面的循环
	while(1){
		Difficulty diff = showDifficultyMenu();
		currentConfig = difficultyConfig[diff];
		
		// 初始化窗口
		initgraph(currentConfig.cols*40, currentConfig.rows*40 + 40, EX_SHOWCONSOLE); // 增加40像素高度用于显示计数器
		loadResource();  // 加载图片资源
		
		// 扫雷地图
		int** map = (int**)malloc(currentConfig.rows * sizeof(int*));
		for (int i = 0; i < currentConfig.rows; i++) 
		{
			map[i] = (int*)malloc(currentConfig.cols * sizeof(int));
		}// 动态分配二维整型数组地图
		
		// 隐藏控制台窗口（仅适用于 GUI 程序）
		HWND hwnd = GetConsoleWindow();
		ShowWindow(hwnd, SW_HIDE); // SW_HIDE 隐藏，SW_SHOW 显示
		
		init(map);
//		show(map);
		
		int shouldReturnToMenu = 0;  // 标记是否需要返回主菜单
		int remainingMines = currentConfig.mines; // 剩余地雷计数器
		
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
					// 只处理游戏区域内的点击
					if (msg.y < currentConfig.rows * 40) {
						mouseMsg(&msg, map, &remainingMines);
						int ret = gameover(map,msg.y/40, msg.x/40);
						
						if(ret == -1 || ret == 1)
						{
							draw(map, remainingMines);
							
							if (ret == -1)
							{
								// 绘制半透明黑色背景（模拟弹窗）
								setfillcolor(RGB(200, 200, 200));
								setbkcolor(RGB(200, 200, 200));
								settextcolor(BLACK);
								if(currentConfig.cols == 10)
								{
									fillrectangle(50, 50, 350,350);
								}
								else if(currentConfig.cols == 16){
									fillrectangle(170,170,470,470);
								}
								else {
									fillrectangle(450,170,750,470);
								}
								
								// 显示失败文字
								settextstyle(24, 0, "微软雅黑");
								outtextxy(currentConfig.cols*20-40, currentConfig.rows*20-120,"游戏失败！");
								
								// 显示两张失败图片
								putimage(currentConfig.cols*20-130, currentConfig.rows*20-60, &img[12]);  // lose1.jpg
								putimage(currentConfig.cols*20+10, currentConfig.rows*20-60, &img[13]); // lose2.jpg
								
								// 显示按钮文字
								outtextxy(currentConfig.cols*20-120, currentConfig.rows*20+100, "再来一次(Y)");
								outtextxy(currentConfig.cols*20+10, currentConfig.rows*20+100, "返回主菜单(N)");
								
								// 等待玩家按键选择
								while (1)
								{
									if (GetAsyncKeyState('Y') & 0x8000) // 按 Y 键重新开始
									{
										init(map);
										remainingMines = currentConfig.mines;
										break;
									}
									else if (GetAsyncKeyState('N') & 0x8000) // 按 N 键返回
									{
										// 标记需要返回主菜单
										shouldReturnToMenu = 1;
										break;
									}
									Sleep(10); // 避免 CPU 占用过高
								}
							}
							else if(ret == 1)
							{
								if (showWinAnimation()) {
									init(map);
									remainingMines = currentConfig.mines;
								} else 
								{
									// 标记需要返回主菜单
									shouldReturnToMenu = 1;
									break;
								}
							}
							// 如果需要返回主菜单，跳出循环
							if (shouldReturnToMenu) break;    
						}
						system("cls");
						// system 函数来自 <stdlib.h> 头文件，它能够将一个字符串参数作为系统命令传递给
						// 操作系统的命令解释器来执行。system("cls"); 中的 "cls" 是 Windows 系统下的命令
						//	，其功能是清空命令提示符窗口（控制台窗口）中的所有内容，使窗口变为空白。
						printf("judege:%d\n", ret);
//						show(map);
					}
					break;
				}
			}    
			// 如果需要返回主菜单，跳出游戏循环
			if (shouldReturnToMenu) break;
			
			// 绘制游戏画面
			draw(map, remainingMines);
			FlushBatchDraw();
		}
		// 释放地图内存
		for (int i = 0; i < currentConfig.rows; i++) {
			free(map[i]);
		}
		free(map);
		
		// 关闭图形窗口
		closegraph();
		
		// 如果不需要返回主菜单(比如玩家点了关闭按钮)，退出程序
		if (!shouldReturnToMenu) {
			break;
		}
	}
	getchar();
	return 0;
}

Difficulty showDifficultyMenu()
{
	// 隐藏控制台窗口（仅适用于 GUI 程序）
	HWND hwnd = GetConsoleWindow();
	ShowWindow(hwnd, SW_HIDE); // SW_HIDE 隐藏，SW_SHOW 显示
	
	initgraph(400, 400, EX_SHOWCONSOLE);
	
	setbkcolor(RGB(240, 240, 240));
	cleardevice();//清屏
	
	settextcolor(BLACK);
	settextstyle(54, 0, "黑体");
	outtextxy(95, 30, "扫雷游戏");
	
	settextstyle(24, 0, "微软雅黑");
	outtextxy(140, 110, "选择难度级别");
	
	setfillcolor(RGB(220, 220, 220));
	setbkcolor(RGB(220,220,220));
	fillrectangle(100, 150, 300, 200);
	fillrectangle(100, 220, 300, 270);
	fillrectangle(100, 290, 300, 340);
	
	settextcolor(BLACK);
	outtextxy(150, 165, "简单 (10x10)");
	outtextxy(150, 235, "中等 (16x16)");
	outtextxy(150, 305, "困难 (16x30)");
	
	ExMessage msg;//点击数据
	while (1) {
		if (peekmessage(&msg, EX_MOUSE)) {
			if (msg.message == WM_LBUTTONDOWN) {
				if (msg.x >= 100 && msg.x <= 300) {
					if (msg.y >= 150 && msg.y <= 200) {
						//点击范围
						closegraph();
						return EASY;
					}
					else if (msg.y >= 220 && msg.y <= 270) {
						closegraph();
						return MEDIUM;
					}
					else if (msg.y >= 290 && msg.y <= 340) {
						closegraph();
						return HARD;
					}
				}
			}
		}
		Sleep(10);// 让当前线程暂停 10 毫秒，避免cpu占用
	}
}

//在控制台打印map
void show(int** map)
{
	int i = 0;
	int k = 0;
	for (i = 0; i < currentConfig.rows; i++)
	{
		for (k = 0; k < currentConfig.cols; k++)
		{
			printf("%3d", map[i][k]);
		}
		printf("\n");
	}
}

// 初始化数据
void init(int** map)
{
	loadResource();//调用函数
	
	// 初始化地图为全0
	for (int i = 0; i < currentConfig.rows; i++) {
		for (int k = 0; k < currentConfig.cols; k++) {
			map[i][k] = 0;
		}
	}
	
	srand((unsigned int)time(NULL));//初始化随机数生成器种子
	//time(NULL) 是 <time.h> 头文件里的一个函数，它会返回当前的系统时间
	// 随机设置十个雷 用-1表示
	
	
	int i = 0;
	while (i < currentConfig.mines)
	{
		int r = rand() % currentConfig.rows;
		int c = rand() % currentConfig.cols;
		if (map[r][c] == 0)
		{
			map[r][c] = -1;  // -1表示地雷
			i++;
		}
	}
	
	// 计算周围雷的数量，把雷周围九宫格的数字加1
	for (int i = 0; i < currentConfig.rows; i++)
	{
		for (int k = 0; k < currentConfig.cols; k++)
		{
			// 找到雷，并遍历雷所在的九宫格
			if (map[i][k] == -1)
			{
				for (int r = i - 1; r <= i + 1; r++)
				{
					for (int c = k - 1; c <= k + 1; c++)
					{
						// 判断是否越界
						if (r >= 0 && r < currentConfig.rows && c >= 0 && c < currentConfig.cols)
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
	for (int i = 0; i < currentConfig.rows; i++)
	{
		for (int k = 0; k < currentConfig.cols; k++)
		{
			map[i][k] += 20;
		}
	}
}

// 绘制
void draw(int** map, int remainingMines)
{
	// 贴图，根据map里面的数字，贴图
	for (int i = 0; i < currentConfig.rows; i++)
	{
		for (int k = 0; k < currentConfig.cols; k++)
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
	
	// 绘制地雷计数器
	drawMineCounter(remainingMines);
}

// 绘制地雷计数器
void drawMineCounter(int remainingMines)
{
	// 绘制计数器背景
	setfillcolor(RGB(220, 220, 220));
	fillrectangle(0, currentConfig.rows * 40, currentConfig.cols * 40, currentConfig.rows * 40 + 40);
	
	// 设置文字样式
	settextcolor(BLACK);
	settextstyle(24, 0, "微软雅黑");
	setbkmode(TRANSPARENT);
	
	// 显示剩余地雷数量
	char mineText[50];
	sprintf(mineText, "剩余地雷: %d", remainingMines);
	outtextxy(10, currentConfig.rows * 40 + 8, mineText);
}

// 鼠标操作数据
void mouseMsg(ExMessage *msg, int** map, int* remainingMines)
{
	// 先根据鼠标的坐标，计算出点击的是哪个格子
	// 左键打开格子，右键标记格子
	int r = msg->y / 40;//ExMessage结构体中的short x; 鼠标的 x 坐标
	int c = msg->x / 40;//short y;鼠标的 y 坐标
	
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
			(*remainingMines)--; // 标记地雷，剩余数量减少
		}
		else if (map[r][c] >= 39)
		{
			map[r][c] -= 20;
			(*remainingMines)++; // 取消标记，剩余数量增加
		}
	}
}

// 点击空白格子，打开周围的格子
void boomblank(int** map, int row, int col)
{
	//判断是不是空白格子
	if (map[row][col] == 0)
	{ 
		for(int r =row-1;r <=row+1; r++)
		{
			for(int c =col-1;c <=col+1; c++)
			{
				if(r >= 0 && r < currentConfig.rows && c >= 0 && c < currentConfig.cols)  //没越界
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
int gameover(int** map,int row,int col)
{
	//点到雷结束，输
	if(map[row][col] == -1 || map[row][col] == 19)
	{
		return -1;
	}
	
	//点到格子结束，赢
	int cnt =0;
	for(int i =0;i < currentConfig.rows;i++)
	{
		for(int k =0;k < currentConfig.cols;k++)
		{
			//统计打开的格子的数量
			if(map[i][k] >= 0 && map[i][k] <= 8)
			{
				cnt++;
			}
		}
	}
	if(currentConfig.rows * currentConfig.cols - currentConfig.mines == cnt)
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
	if(currentConfig.cols == 10)
	{
		fillrectangle(50, 50, 350,350);
	}
	else if(currentConfig.cols == 16){
		fillrectangle(170,170,470,470);
	}
	else {
		fillrectangle(450,170,750,470);
	}
	
	
	// 显示文字
	settextcolor(RGB(250, 250, 250));
	settextstyle(24, 0, "微软雅黑");
	outtextxy(currentConfig.cols*20-40, currentConfig.rows*20-120, "胜利啦！");
	
	// 动画循环
	while (1) {
		// 分别计算两个动画的当前帧
		frame1 = (GetTickCount() - startTime) / 100 % WIN_ANIM_FRAMES;  // 呆菇：17帧
		frame2 = (GetTickCount() - startTime) / 40 % WIN_ANIM_FRAMES2; // 巫师：44帧
		
		// 绘制动画帧
		putimage(currentConfig.cols*20-130, currentConfig.rows*20-60, &winAnim1[frame1]);  // 呆菇
		putimage(currentConfig.cols*20+10, currentConfig.rows*20-60, &winAnim2[frame2]); // 巫师
		
		// 显示按钮提示
		settextcolor(RGB(250, 250, 250));
		outtextxy(currentConfig.cols*20-120, currentConfig.rows*20+100, "再来一局(Y)");
		outtextxy(currentConfig.cols*20+10, currentConfig.rows*20+100, "返回主菜单(N)");
		
		// 检查按键
		if (GetAsyncKeyState('Y') & 0x8000) return true;
		if (GetAsyncKeyState('N') & 0x8000) return false;
		
		FlushBatchDraw();
	}
}
