/*
 * Delaunay三角形逐点插入法
 *
 */
#pragma warning(disable:4996)
#include<stdio.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>
#include<gl/glut.h>

//#define SQUARES_NUM   1000
#define PI                    	 		  3.1415926
//#define STOREHOUSEHEIGHT 					1605.0 		//设备到底部高度单位cm
#define DIAMETER							1840.0
#define DEVICELENG							39.0		//边距加轴距
//#define LENG								20.0		
#define INTERVAL							100.0		//点间隔

struct point_coord {
	double locate_x;
	double locate_y;
	double height;
	double pointheight;
};

struct point_info {
	int angle_ver;
	int angle_hor;
	int distance;
};

struct tri {
	int    p1;
	int    p2;
	int    p3;
	double cx;
	double cy;
	double radius2;
	double avePointHeight;
	double area;
};

struct line_info {
	int startpoint;
	int endpoint;
};

struct point_coord point_data[1000] = { 0 };	//点的坐标、高度
struct point_coord incircpoint[1000] = { 0 }; //圆环上点的坐标
struct point_info point_inf[1000] = { 0 };	//原始点的数据

struct tri delaunay_tri[3000] = { 0 };  //存放delaunay三角形信息的数组  
struct tri temp_tri[1000] = { 0 }; //存放临时三角形
struct tri final_tri[1000] = { 0 }; //存放最终三角形，形成三角形网
struct line_info edge_buf[1000] = { 0 }; //存放临时边

static int data_buf[1000] = { 0 }; //存放点的数据
static int total_point = 0;	//点的总数
//static int buf[200] = {0};
//static double radius = 0;
//static int linenum = 0;
//static int trinum = 0;
static int delaunaynum = 0;
static int finalnum = 0;
static int circpointnum = 0;

//读取文本数据
int read_data(void)
{
	FILE *fp = NULL;
	char *pchbuf = NULL;
	char tmp[5] = { 0 };
	int nLen, cnt = 0;
	int i, j = 0, k = 0;
	//int num[8] = {0};
	fp = fopen("out.txt", "r");
	fseek(fp, 0, SEEK_END);
	nLen = ftell(fp);
	rewind(fp);

	//动态申请空间
	pchbuf = (char*)malloc(sizeof(char)*nLen + 1);
	if (!pchbuf)
	{
		printf("内存不够!\n");
		return -1;
	}
	//读取文件内容的长度
	nLen = fread(pchbuf, sizeof(char), nLen, fp);
	fclose(fp);

	pchbuf[nLen] = '\0'; //添加字符串结尾标志
	printf("read char\n");
	printf("%s\n", pchbuf); //把读取的内容输出到屏幕看看
	printf("------------------------------------\n");
	printf("change char to int\n");
	for (i = 0; i < nLen; i++)
	{
		printf("%c", *(pchbuf + i));
		tmp[j++] = (*(pchbuf + i));

		if ((*(pchbuf + i)) == ';' || (*(pchbuf + i)) == '\0' || (*(pchbuf + i)) == '+')
		{
			tmp[j - 1] = '\0';
			cnt++;
			data_buf[k++] = atoi(tmp);
			j = 0;
		}
	}
	printf("\n");
	free(pchbuf);
	total_point = cnt / 3;
	printf("total_point = %d\n", total_point);
	printf("\n");
	return 0;
}

/*
 * 得到每一个点信息
 * 读取的数据格式 vertical angle + horizon angle + distance
 */
void get_point_info(void)
{
	int i = 0, j = 0, k = 0;
	for (i = 0; i < total_point; i++)
	{
		for (j = 0; j < 3; j++)
		{
			if (j == 0)
			{
				point_inf[i].angle_ver = data_buf[k++];
			}
			else if (j == 1)
			{
				point_inf[i].angle_hor = data_buf[k++];
			}
			else
			{
				point_inf[i].distance = data_buf[k++];
			}
		}
	}
}

/* 角度转化为x坐标 以设备为原点建立二维坐标 */
double angle_x(struct point_info point)
{
	double L;
	double Xcoor;

	L = (double)(sin(PI / 180 * point.angle_ver) * point.distance) + DEVICELENG; /* 水平长度 */
	Xcoor = (double)(cos(PI / 180 * point.angle_hor) * L);
	return Xcoor;
}


//角度转化为y坐标
double angle_y(struct point_info point)
{
	double L;
	double Ycoor;

	L = (double)(sin(PI / 180 * point.angle_ver) * point.distance) + DEVICELENG;
	Ycoor = (double)(sin(PI / 180 * point.angle_hor) * L);
	return Ycoor;
}

//点的垂直距离
double point_vertialHeight(struct point_info point)
{
	double height;
	height = (double)(cos(PI / 180 * point.angle_ver) * point.distance);
	return height;
}


//获取每一个点坐标的信息
void get_point_coor(void)
{
	int i;
	//	int k = 200;

	for (i = 0; i < total_point; i++)
	{
		point_data[i].locate_x = angle_x(point_inf[i]);
		point_data[i].locate_y = angle_y(point_inf[i]);
		point_data[i].height = point_vertialHeight(point_inf[i]);
		point_data[i].pointheight = 1600.0 - point_data[i].height;
	}
}

/* 获得圆上点y的坐标 圆心坐标为(radius, 0)*/
double getylocate(double xlocate, double radius)
{
	double ylocate;
	ylocate = sqrt((radius * radius) - (xlocate - radius) * (xlocate - radius));
	return ylocate;
}

/*
 * 根据圆柱仓库构造一圈在圆环上的点
 * x坐标以 INTERVAL 的间隔增加
 */
void make_cirpoint(double diameter)
{
	int i;
	double locatey;
	for (i = 0; (i*INTERVAL) < diameter; i++)
	{
		if (i == 0)
		{
			locatey = getylocate(i*INTERVAL, diameter / 2);
			incircpoint[circpointnum].locate_x = i * INTERVAL;
			incircpoint[circpointnum].locate_y = locatey;
			circpointnum += 1;

		}
		else
		{
			/* x!=0 y值有正负两个值 */
			locatey = getylocate(i*INTERVAL, diameter / 2);
			incircpoint[circpointnum].locate_x = i * INTERVAL;
			incircpoint[circpointnum].locate_y = locatey;
			circpointnum += 1;
			incircpoint[circpointnum].locate_x = i * INTERVAL;
			incircpoint[circpointnum].locate_y = -locatey;
			circpointnum += 1;
		}
	}
}

/* 根据totalpoint中点的数据，获得圆上每一个点的近似高度数据 */
void getcircpointinfo(void)
{
	int i, j;
	double distance;
	double tempdistance = 10000;
	for (i = 0; i < circpointnum; i++)
	{
		for (j = 0; j < total_point; j++)/* 找距离圆上点最近的点 */
		{
			distance = sqrt((incircpoint[i].locate_x - point_data[j].locate_x) * (incircpoint[i].locate_x - point_data[j].locate_x)\
				+ (incircpoint[i].locate_y - point_data[j].locate_y) * (incircpoint[i].locate_y - point_data[j].locate_y));
			if (tempdistance > distance)
			{
				incircpoint[i].pointheight = point_data[j].pointheight;
			}
		}
	}
}

/* 把点加到总点的数组里面 */
void addpointtototal(void)
{
	int i, j;
	for (i = total_point, j = 0; (i < (total_point + circpointnum)) && (j < circpointnum); i++, j++)
	{
		point_data[i] = incircpoint[j];
	}
	total_point += circpointnum;
}

/* 三角形面积计算 */
double tri_area(struct tri triangle)
{
	/* 海伦公式 */
	double a, b, c;
	double halfcirc; /* 半周长 */
	double area;

	a = (sqrt((point_data[triangle.p1].locate_x - point_data[triangle.p2].locate_x) * (point_data[triangle.p1].locate_x - point_data[triangle.p2].locate_x)\
		+ (point_data[triangle.p1].locate_y - point_data[triangle.p2].locate_y) * (point_data[triangle.p1].locate_y - point_data[triangle.p2].locate_y))) / 100.0;
	b = (sqrt((point_data[triangle.p1].locate_x - point_data[triangle.p3].locate_x) * (point_data[triangle.p1].locate_x - point_data[triangle.p3].locate_x)\
		+ (point_data[triangle.p1].locate_y - point_data[triangle.p3].locate_y) * (point_data[triangle.p1].locate_y - point_data[triangle.p3].locate_y))) / 100.0;
	c = (sqrt((point_data[triangle.p2].locate_x - point_data[triangle.p3].locate_x) * (point_data[triangle.p2].locate_x - point_data[triangle.p3].locate_x)\
		+ (point_data[triangle.p2].locate_y - point_data[triangle.p3].locate_y) * (point_data[triangle.p2].locate_y - point_data[triangle.p3].locate_y))) / 100.0;

	halfcirc = (a + b + c) / 2.0;
	area = sqrt(halfcirc * (halfcirc - a) * (halfcirc - b) * (halfcirc - c));
	return area;
}

/* 对坐标点沿x轴从小到大排序 */
void sortpoint_x(struct point_coord point[])
{
	int i, j;
	struct point_coord temp;
	for (i = 0; i < total_point; i++)
	{
		for (j = i + 1; j < total_point; j++)
		{
			if (point[i].locate_x > point[j].locate_x)
			{
				temp = point[i];
				point[i] = point[j];
				point[j] = temp;
			}
		}
	}
}

/* 构造一个超级三角形 必须保证三角形的底大于高
 *     C
 *
 *  A     B
 *
 * 把点放入顶点数组的末尾，
 * 把超级三角形放入 temptri
 */
void creatsupertri(void)
{
	int i;
	double minx = point_data[0].locate_x;
	double miny = point_data[0].locate_y;
	double maxx = point_data[0].locate_x;
	double maxy = point_data[0].locate_y;
	double dx;
	double dy;
	double wide;
	double midx, midy;
	struct point_coord a;
	struct point_coord b;
	struct point_coord c;

	for (i = 0; i < total_point; i++)
	{
		if (point_data[i].locate_x < minx)
		{
			minx = point_data[i].locate_x;
		}
		else if (point_data[i].locate_x > maxx)
		{
			maxx = point_data[i].locate_x;
		}
		if (point_data[i].locate_y < miny)
		{
			miny = point_data[i].locate_y;
		}
		else if (point_data[i].locate_y > maxy)
		{
			maxy = point_data[i].locate_y;
		}
	}
	dx = maxx - minx;
	dy = maxy - miny;
	midx = minx + dx * 0.5;
	midy = miny + dy * 0.5;
	wide = dx > dy ? dx : dy;
	a.locate_x = midx - wide * 3.0;
	a.locate_y = midy - wide;
	b.locate_x = midx + wide * 3.0;
	b.locate_y = midy - wide;
	c.locate_x = midx;
	c.locate_y = midy + wide * 2.0;
	/* 把点放在顶点末尾 */
	point_data[total_point] = a;
	point_data[total_point + 1] = b;
	point_data[total_point + 2] = c;
	temp_tri[0].p1 = total_point;
	temp_tri[0].p2 = total_point + 1;
	temp_tri[0].p3 = total_point + 2;

}

/* 计算三角形外接圆 */
void circumcircle(struct tri *triangle)
{
	/* 海伦公式 */
	double t1, t2, t3, dt;

	t1 = point_data[triangle->p1].locate_x * point_data[triangle->p1].locate_x + point_data[triangle->p1].locate_y * point_data[triangle->p1].locate_y;
	t2 = point_data[triangle->p2].locate_x * point_data[triangle->p2].locate_x + point_data[triangle->p2].locate_y * point_data[triangle->p2].locate_y;
	t3 = point_data[triangle->p3].locate_x * point_data[triangle->p3].locate_x + point_data[triangle->p3].locate_y * point_data[triangle->p3].locate_y;
	dt = 2.0 * (point_data[triangle->p1].locate_x * point_data[triangle->p2].locate_y + point_data[triangle->p2].locate_x * point_data[triangle->p3].locate_y + \
		point_data[triangle->p3].locate_x * point_data[triangle->p1].locate_y - point_data[triangle->p1].locate_y * point_data[triangle->p2].locate_x - \
		point_data[triangle->p2].locate_y * point_data[triangle->p3].locate_x - point_data[triangle->p3].locate_y * point_data[triangle->p1].locate_x);

	triangle->cx = ((point_data[triangle->p3].locate_y - point_data[triangle->p1].locate_y) * (t2 - t1) - \
		(point_data[triangle->p2].locate_y - point_data[triangle->p1].locate_y) * (t3 - t1)) / dt;
	triangle->cy = ((point_data[triangle->p2].locate_x - point_data[triangle->p1].locate_x) * (t3 - t1) - \
		(point_data[triangle->p3].locate_x - point_data[triangle->p1].locate_x) * (t2 - t1)) / dt;
	triangle->radius2 = (point_data[triangle->p1].locate_x - triangle->cx) * (point_data[triangle->p1].locate_x - triangle->cx) + (point_data[triangle->p1].locate_y - triangle->cy) * (point_data[triangle->p1].locate_y - triangle->cy);
}

/* 去掉重复的边  返回剩余的边个数 */
int removedoubleedge(struct line_info edgebuf[], int num)
{
	int i, j;
	int total = 0;
	int flag = 0; /* 该边是否重复 */
	for (i = 0; i < num; i++)
	{
		for (j = i + 1; j < num; j++)
		{
			if (((edgebuf[i].startpoint == edgebuf[j].startpoint) && (edgebuf[i].endpoint == edgebuf[j].endpoint))\
				|| ((edgebuf[i].startpoint == edgebuf[j].endpoint) && (edgebuf[i].endpoint == edgebuf[j].startpoint)))
			{
				memset(edgebuf + j, 0, sizeof(struct line_info));
				flag = 1;
			}
		}
		/* 如果这条边是重复的 把这条边也删除了 */
		if (flag == 1)
		{
			memset(edgebuf + i, 0, sizeof(struct line_info));
		}
		flag = 0;
	}
	i = 0;
	while (i < (num - total))
	{
		if ((edgebuf[i].endpoint == 0) && (edgebuf[i].startpoint == 0))
		{
			for (j = i; j < (num - 1); j++)
			{
				edgebuf[j] = edgebuf[j + 1];
			}
			i--;
			total++;
		}
		i++;
	}
	total = num - total;
	return total;
}


/* 把 temp_tri 初始化为零的三角形去掉 返回三角形个数 */
int removezerotri(struct tri triarr[], int trinum)
{
	int i, j;
	int cnt = 0;
	for (i = 0; i < (trinum - cnt); i++)
	{
		if ((triarr[i].p1 == 0) && (triarr[i].p2 == 0) && (triarr[i].p3 == 0))
		{
			for (j = i; j < (trinum - 1); j++)
			{
				triarr[j] = triarr[j + 1];
			}
			i -= 1;
			cnt++;
		}
	}
	cnt = trinum - cnt;
	return cnt;
}

/* 生成三角形网格 */
void creat_mesh(void)
{
	int i, j, k;
	int m, n, p;
	int edgenum = 0;
	int temptrinum = 0;
	double dist2;
	double dx;
	double dy;
	creatsupertri();
	temptrinum = 1;
	/* 遍历所有的点 */
	for (i = 0; i < total_point; i++)
	{
		/* 遍历当前点，与每一个三角形外接圆的关系 temptrinum */
		for (j = 0; j < temptrinum; j++)
		{
			circumcircle(&temp_tri[j]); /* 计算三角形外接圆 */
			dist2 = (point_data[i].locate_x - temp_tri[j].cx) * (point_data[i].locate_x - temp_tri[j].cx) + (point_data[i].locate_y - temp_tri[j].cy) * (point_data[i].locate_y - temp_tri[j].cy);
			dx = point_data[i].locate_x - temp_tri[j].cx;
			dy = point_data[i].locate_y - temp_tri[j].cy;

			if (dist2 < temp_tri[j].radius2) /* 1、点在三角形外接圆内 */
			{
				/*
				 * 这个三角形不是delaunay三角形
				 * 		把三条边保存到 edge_buf 中
				 * 在 temp_tri 中去掉该三角形
				 */
				for (k = 0; k < 3; k++) /* 把一个三角形的边存起来 */
				{
					if (k == 0)
					{
						edge_buf[edgenum].startpoint = temp_tri[j].p1;
						edge_buf[edgenum].endpoint = temp_tri[j].p2;
						edgenum += 1;
					}
					else if (k == 1)
					{
						edge_buf[edgenum].startpoint = temp_tri[j].p1;
						edge_buf[edgenum].endpoint = temp_tri[j].p3;
						edgenum += 1;
					}
					else
					{
						edge_buf[edgenum].startpoint = temp_tri[j].p2;
						edge_buf[edgenum].endpoint = temp_tri[j].p3;
						edgenum += 1;
					}
				}
				/* 在 temp_tri 中去掉这个三角形 - 把这一个初始化为0*/
				memset(temp_tri + j, 0, sizeof(struct tri));
			}
			else if ((dx > 0) && ((dx * dx) > temp_tri[j].radius2))
			{/* 2、点在外接圆的右侧 */
				/*
				 * 该三角形是delaunay三角形，保存到 delaunay_tri
				 * 在 temp_tri 中去掉该三角形
				 */
				 /* 在 temp_tri 中去掉这个三角形 - 把这一个初始化为0*/
				delaunay_tri[delaunaynum] = temp_tri[j];
				delaunaynum++;
				/* 去掉这个三角形 */
				memset(temp_tri + j, 0, sizeof(struct tri));
			}
			/*
			else
			{
				// 3、点既不在外接圆内，也不再外接圆右边，不作任何操作
			}
			*/
		}

		/* 获得剩余的三角形 */
		temptrinum = removezerotri(temp_tri, temptrinum);
		/* 把 edge_buf 中的重复边去掉(重复的包括重复本身) */
		edgenum = removedoubleedge(edge_buf, edgenum);
		/*
		 * 将 edge_buf 中的边与当前的点进行组合成若干三角形
		 * 并保存至 temp_tri 中
		 */
		for (m = 0; m < edgenum; m++)
		{
			temp_tri[temptrinum].p1 = i;
			temp_tri[temptrinum].p2 = edge_buf[m].startpoint;
			temp_tri[temptrinum].p3 = edge_buf[m].endpoint;
			temptrinum += 1;
		}
		/* 清空 edge_buf 里面的边 */
		memset(edge_buf, 0, sizeof(edge_buf));
		edgenum = 0;
	}

	/* 获得剩余的三角形 */
	temptrinum = removezerotri(temp_tri, temptrinum);

	/* 把delaunay三角形和 temp_tri 合并 */
	for (n = delaunaynum, p = 0; (n < (delaunaynum + temptrinum)) && (p < temptrinum); n++, p++)
	{
		delaunay_tri[n] = temp_tri[p];
	}
	delaunaynum += temptrinum;
}

/* 三角网的构建 */
void build_trinet(void)
{
	/*
	 *　除去与超级三角形顶点有关的三角形
	 */
	int i;
	for (i = 0; i < delaunaynum; i++)
	{
		if ((delaunay_tri[i].p1 != total_point) && (delaunay_tri[i].p1 != total_point + 1) && (delaunay_tri[i].p1 != total_point + 2))
		{
			if ((delaunay_tri[i].p2 != total_point) && (delaunay_tri[i].p2 != total_point + 1) && (delaunay_tri[i].p2 != total_point + 2))
			{
				if ((delaunay_tri[i].p3 != total_point) && (delaunay_tri[i].p3 != total_point + 1) && (delaunay_tri[i].p3 != total_point + 2))
				{
					final_tri[finalnum] = delaunay_tri[i];
					finalnum += 1;
				}
			}
		}
	}
	printf("finalnum = %d\n", finalnum);
	for (i = 0; i < finalnum; i++)
	{
		printf("p1 = %d,p2 = %d,p3 = %d\n", final_tri[i].p1, final_tri[i].p2, final_tri[i].p3);
	}
}

/*获取三角形相关信息*/
void get_triinfo(void)
{
	int i;
	for (i = 0; i < finalnum; i++)
	{
		final_tri[i].avePointHeight = ((point_data[final_tri[i].p1].pointheight + point_data[final_tri[i].p2].pointheight + point_data[final_tri[i].p3].pointheight) / 3.0) / 100.0;
		final_tri[i].area = tri_area(final_tri[i]);
	}
}

/* 计算体积 */
double calculate_volume(void)
{
	double volume = 0;
	double r1 = 0;
	double r2 = 0;
	int i = 0;
	for (i = 0; i < finalnum; i++)
	{
		volume += final_tri[i].avePointHeight * final_tri[i].area;
	}
	return volume;
}

void renderWindow(void)
{
	int i;
	glClear(GL_COLOR_BUFFER_BIT);     //当前背景色填充窗口
	glColor3f(1, 0, 0);               //颜色:RGB.此处R:1,为红色.
//	glBegin(GL_TRIANGLES);            //开始:显示三角形面
//	glBegin(GL_POINTS);				  //显示点
	for (i = 0; i < finalnum; i++)
	{
		glBegin(GL_LINE_LOOP);		   	  //画点
	/*
		glVertex2f(-0.5f, -0.5f);         //三个顶点坐标:
		glVertex2d(0.5f, -0.5f);
		glVertex2d(-0.0f, 0.5f);
	*/
		/* 坐标显示的范围是[ -1, 1 ] */
		glVertex2d(point_data[final_tri[i].p1].locate_x / 2000, point_data[final_tri[i].p1].locate_y / 2000);
		glVertex2d(point_data[final_tri[i].p2].locate_x / 2000, point_data[final_tri[i].p2].locate_y / 2000);
		glVertex2d(point_data[final_tri[i].p3].locate_x / 2000, point_data[final_tri[i].p3].locate_y / 2000);

//		glVertex2d(point_data[final_tri[i].p1].locate_x / 100, point_data[final_tri[i].p1].locate_y / 100);
//		glVertex2d(point_data[final_tri[i].p2].locate_x / 100, point_data[final_tri[i].p2].locate_y / 100);
//		glVertex2d(point_data[final_tri[i].p3].locate_x / 100, point_data[final_tri[i].p3].locate_y / 100);
		glEnd();                          //结束
	}
	//glEnd();                          //结束
	glFlush();                        //输出缓冲区 
}

void renderWindow1(void)
{
	glClear(GL_COLOR_BUFFER_BIT);                           //当前背景色填充窗口
	glColor3f(1, 0, 0);                                       //颜色:RGB.此处R:1,为红色.
//	glBegin(GL_TRIANGLES);                                  //开始:三角形
//	glBegin(GL_POINTS);				
	glBegin(GL_LINE_LOOP);				//画点
/*
	glVertex2f(-0.5f, -0.5f);                               //三个顶点坐标:
	glVertex2d(0.5f, -0.5f);
	glVertex2d(-0.0f, 0.5f);
*/
	glVertex2f(0.0f, 0.0f);                               //三个顶点坐标:
	glVertex2d(0.0f, 0.5f);
	glVertex2d(0.5f, 0.0f);
	glEnd();                                                //结束
	glFlush();                                              //输出缓冲区 
}

int main(int argc,char *argv[])
{
	double volume = 0;
	read_data();
	get_point_info();
	get_point_coor();
	make_cirpoint(DIAMETER);
	getcircpointinfo();
	addpointtototal();
	sortpoint_x(point_data);
	//	creatsupertri();
	creat_mesh();
	build_trinet();
	get_triinfo();
	volume = calculate_volume();
	printf("volume = %f\n", volume);

	glutInit(&argc, argv);                                  //初始化glut: 接收主函数的参数
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);            //显示模式：颜色&缓冲
	glutInitWindowPosition(0, 0);                           //窗口相对屏幕位置(0, 0)电脑左上角
	glutInitWindowSize(720, 720);                           //画面窗口大小
	glutCreateWindow("Tri Net!");                           //创建窗口: 标题
	glutDisplayFunc(&renderWindow);                         //显示函数
	glutMainLoop();                                         //循环

	return 0;
}

