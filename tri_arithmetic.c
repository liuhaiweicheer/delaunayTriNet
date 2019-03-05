/*
 * Delaunay�����������뷨
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
//#define STOREHOUSEHEIGHT 					1605.0 		//�豸���ײ��߶ȵ�λcm
#define DIAMETER							1840.0
#define DEVICELENG							39.0		//�߾�����
//#define LENG								20.0		
#define INTERVAL							100.0		//����

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

struct point_coord point_data[1000] = { 0 };	//������ꡢ�߶�
struct point_coord incircpoint[1000] = { 0 }; //Բ���ϵ������
struct point_info point_inf[1000] = { 0 };	//ԭʼ�������

struct tri delaunay_tri[3000] = { 0 };  //���delaunay��������Ϣ������  
struct tri temp_tri[1000] = { 0 }; //�����ʱ������
struct tri final_tri[1000] = { 0 }; //������������Σ��γ���������
struct line_info edge_buf[1000] = { 0 }; //�����ʱ��

static int data_buf[1000] = { 0 }; //��ŵ������
static int total_point = 0;	//�������
//static int buf[200] = {0};
//static double radius = 0;
//static int linenum = 0;
//static int trinum = 0;
static int delaunaynum = 0;
static int finalnum = 0;
static int circpointnum = 0;

//��ȡ�ı�����
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

	//��̬����ռ�
	pchbuf = (char*)malloc(sizeof(char)*nLen + 1);
	if (!pchbuf)
	{
		printf("�ڴ治��!\n");
		return -1;
	}
	//��ȡ�ļ����ݵĳ���
	nLen = fread(pchbuf, sizeof(char), nLen, fp);
	fclose(fp);

	pchbuf[nLen] = '\0'; //����ַ�����β��־
	printf("read char\n");
	printf("%s\n", pchbuf); //�Ѷ�ȡ�������������Ļ����
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
 * �õ�ÿһ������Ϣ
 * ��ȡ�����ݸ�ʽ vertical angle + horizon angle + distance
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

/* �Ƕ�ת��Ϊx���� ���豸Ϊԭ�㽨����ά���� */
double angle_x(struct point_info point)
{
	double L;
	double Xcoor;

	L = (double)(sin(PI / 180 * point.angle_ver) * point.distance) + DEVICELENG; /* ˮƽ���� */
	Xcoor = (double)(cos(PI / 180 * point.angle_hor) * L);
	return Xcoor;
}


//�Ƕ�ת��Ϊy����
double angle_y(struct point_info point)
{
	double L;
	double Ycoor;

	L = (double)(sin(PI / 180 * point.angle_ver) * point.distance) + DEVICELENG;
	Ycoor = (double)(sin(PI / 180 * point.angle_hor) * L);
	return Ycoor;
}

//��Ĵ�ֱ����
double point_vertialHeight(struct point_info point)
{
	double height;
	height = (double)(cos(PI / 180 * point.angle_ver) * point.distance);
	return height;
}


//��ȡÿһ�����������Ϣ
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

/* ���Բ�ϵ�y������ Բ������Ϊ(radius, 0)*/
double getylocate(double xlocate, double radius)
{
	double ylocate;
	ylocate = sqrt((radius * radius) - (xlocate - radius) * (xlocate - radius));
	return ylocate;
}

/*
 * ����Բ���ֿ⹹��һȦ��Բ���ϵĵ�
 * x������ INTERVAL �ļ������
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
			/* x!=0 yֵ����������ֵ */
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

/* ����totalpoint�е�����ݣ����Բ��ÿһ����Ľ��Ƹ߶����� */
void getcircpointinfo(void)
{
	int i, j;
	double distance;
	double tempdistance = 10000;
	for (i = 0; i < circpointnum; i++)
	{
		for (j = 0; j < total_point; j++)/* �Ҿ���Բ�ϵ�����ĵ� */
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

/* �ѵ�ӵ��ܵ���������� */
void addpointtototal(void)
{
	int i, j;
	for (i = total_point, j = 0; (i < (total_point + circpointnum)) && (j < circpointnum); i++, j++)
	{
		point_data[i] = incircpoint[j];
	}
	total_point += circpointnum;
}

/* ������������� */
double tri_area(struct tri triangle)
{
	/* ���׹�ʽ */
	double a, b, c;
	double halfcirc; /* ���ܳ� */
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

/* ���������x���С�������� */
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

/* ����һ������������ ���뱣֤�����εĵ״��ڸ�
 *     C
 *
 *  A     B
 *
 * �ѵ���붥�������ĩβ��
 * �ѳ��������η��� temptri
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
	/* �ѵ���ڶ���ĩβ */
	point_data[total_point] = a;
	point_data[total_point + 1] = b;
	point_data[total_point + 2] = c;
	temp_tri[0].p1 = total_point;
	temp_tri[0].p2 = total_point + 1;
	temp_tri[0].p3 = total_point + 2;

}

/* �������������Բ */
void circumcircle(struct tri *triangle)
{
	/* ���׹�ʽ */
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

/* ȥ���ظ��ı�  ����ʣ��ı߸��� */
int removedoubleedge(struct line_info edgebuf[], int num)
{
	int i, j;
	int total = 0;
	int flag = 0; /* �ñ��Ƿ��ظ� */
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
		/* ������������ظ��� ��������Ҳɾ���� */
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


/* �� temp_tri ��ʼ��Ϊ���������ȥ�� ���������θ��� */
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

/* �������������� */
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
	/* �������еĵ� */
	for (i = 0; i < total_point; i++)
	{
		/* ������ǰ�㣬��ÿһ�����������Բ�Ĺ�ϵ temptrinum */
		for (j = 0; j < temptrinum; j++)
		{
			circumcircle(&temp_tri[j]); /* �������������Բ */
			dist2 = (point_data[i].locate_x - temp_tri[j].cx) * (point_data[i].locate_x - temp_tri[j].cx) + (point_data[i].locate_y - temp_tri[j].cy) * (point_data[i].locate_y - temp_tri[j].cy);
			dx = point_data[i].locate_x - temp_tri[j].cx;
			dy = point_data[i].locate_y - temp_tri[j].cy;

			if (dist2 < temp_tri[j].radius2) /* 1���������������Բ�� */
			{
				/*
				 * ��������β���delaunay������
				 * 		�������߱��浽 edge_buf ��
				 * �� temp_tri ��ȥ����������
				 */
				for (k = 0; k < 3; k++) /* ��һ�������εıߴ����� */
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
				/* �� temp_tri ��ȥ����������� - ����һ����ʼ��Ϊ0*/
				memset(temp_tri + j, 0, sizeof(struct tri));
			}
			else if ((dx > 0) && ((dx * dx) > temp_tri[j].radius2))
			{/* 2���������Բ���Ҳ� */
				/*
				 * ����������delaunay�����Σ����浽 delaunay_tri
				 * �� temp_tri ��ȥ����������
				 */
				 /* �� temp_tri ��ȥ����������� - ����һ����ʼ��Ϊ0*/
				delaunay_tri[delaunaynum] = temp_tri[j];
				delaunaynum++;
				/* ȥ����������� */
				memset(temp_tri + j, 0, sizeof(struct tri));
			}
			/*
			else
			{
				// 3����Ȳ������Բ�ڣ�Ҳ�������Բ�ұߣ������κβ���
			}
			*/
		}

		/* ���ʣ��������� */
		temptrinum = removezerotri(temp_tri, temptrinum);
		/* �� edge_buf �е��ظ���ȥ��(�ظ��İ����ظ�����) */
		edgenum = removedoubleedge(edge_buf, edgenum);
		/*
		 * �� edge_buf �еı��뵱ǰ�ĵ������ϳ�����������
		 * �������� temp_tri ��
		 */
		for (m = 0; m < edgenum; m++)
		{
			temp_tri[temptrinum].p1 = i;
			temp_tri[temptrinum].p2 = edge_buf[m].startpoint;
			temp_tri[temptrinum].p3 = edge_buf[m].endpoint;
			temptrinum += 1;
		}
		/* ��� edge_buf ����ı� */
		memset(edge_buf, 0, sizeof(edge_buf));
		edgenum = 0;
	}

	/* ���ʣ��������� */
	temptrinum = removezerotri(temp_tri, temptrinum);

	/* ��delaunay�����κ� temp_tri �ϲ� */
	for (n = delaunaynum, p = 0; (n < (delaunaynum + temptrinum)) && (p < temptrinum); n++, p++)
	{
		delaunay_tri[n] = temp_tri[p];
	}
	delaunaynum += temptrinum;
}

/* �������Ĺ��� */
void build_trinet(void)
{
	/*
	 *����ȥ�볬�������ζ����йص�������
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

/*��ȡ�����������Ϣ*/
void get_triinfo(void)
{
	int i;
	for (i = 0; i < finalnum; i++)
	{
		final_tri[i].avePointHeight = ((point_data[final_tri[i].p1].pointheight + point_data[final_tri[i].p2].pointheight + point_data[final_tri[i].p3].pointheight) / 3.0) / 100.0;
		final_tri[i].area = tri_area(final_tri[i]);
	}
}

/* ������� */
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
	glClear(GL_COLOR_BUFFER_BIT);     //��ǰ����ɫ��䴰��
	glColor3f(1, 0, 0);               //��ɫ:RGB.�˴�R:1,Ϊ��ɫ.
//	glBegin(GL_TRIANGLES);            //��ʼ:��ʾ��������
//	glBegin(GL_POINTS);				  //��ʾ��
	for (i = 0; i < finalnum; i++)
	{
		glBegin(GL_LINE_LOOP);		   	  //����
	/*
		glVertex2f(-0.5f, -0.5f);         //������������:
		glVertex2d(0.5f, -0.5f);
		glVertex2d(-0.0f, 0.5f);
	*/
		/* ������ʾ�ķ�Χ��[ -1, 1 ] */
		glVertex2d(point_data[final_tri[i].p1].locate_x / 2000, point_data[final_tri[i].p1].locate_y / 2000);
		glVertex2d(point_data[final_tri[i].p2].locate_x / 2000, point_data[final_tri[i].p2].locate_y / 2000);
		glVertex2d(point_data[final_tri[i].p3].locate_x / 2000, point_data[final_tri[i].p3].locate_y / 2000);

//		glVertex2d(point_data[final_tri[i].p1].locate_x / 100, point_data[final_tri[i].p1].locate_y / 100);
//		glVertex2d(point_data[final_tri[i].p2].locate_x / 100, point_data[final_tri[i].p2].locate_y / 100);
//		glVertex2d(point_data[final_tri[i].p3].locate_x / 100, point_data[final_tri[i].p3].locate_y / 100);
		glEnd();                          //����
	}
	//glEnd();                          //����
	glFlush();                        //��������� 
}

void renderWindow1(void)
{
	glClear(GL_COLOR_BUFFER_BIT);                           //��ǰ����ɫ��䴰��
	glColor3f(1, 0, 0);                                       //��ɫ:RGB.�˴�R:1,Ϊ��ɫ.
//	glBegin(GL_TRIANGLES);                                  //��ʼ:������
//	glBegin(GL_POINTS);				
	glBegin(GL_LINE_LOOP);				//����
/*
	glVertex2f(-0.5f, -0.5f);                               //������������:
	glVertex2d(0.5f, -0.5f);
	glVertex2d(-0.0f, 0.5f);
*/
	glVertex2f(0.0f, 0.0f);                               //������������:
	glVertex2d(0.0f, 0.5f);
	glVertex2d(0.5f, 0.0f);
	glEnd();                                                //����
	glFlush();                                              //��������� 
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

	glutInit(&argc, argv);                                  //��ʼ��glut: �����������Ĳ���
	glutInitDisplayMode(GLUT_RGB | GLUT_SINGLE);            //��ʾģʽ����ɫ&����
	glutInitWindowPosition(0, 0);                           //���������Ļλ��(0, 0)�������Ͻ�
	glutInitWindowSize(720, 720);                           //���洰�ڴ�С
	glutCreateWindow("Tri Net!");                           //��������: ����
	glutDisplayFunc(&renderWindow);                         //��ʾ����
	glutMainLoop();                                         //ѭ��

	return 0;
}

