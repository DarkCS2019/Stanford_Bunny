#include <iostream>
#include<GL/glut.h>
#include<stdlib.h>
#include<fstream>
#include<string>
#include<Windows.h>
#include<cstring>
#include <sstream>
#include<vector>
using namespace std;

#pragma region 坐标系
//    y(z)
//    |
//    |
//     -----x(y)
//   /
// /
//z(x)
#pragma endregion

#pragma region 类
struct point
{
	GLfloat x;
	GLfloat y;
	GLfloat z;
	point(double x1, double y1, double z1) { x = x1, y = y1, z = z1; }
};
struct triangle
{
	int p1;
	int p2;
	int p3;
	triangle(int x1, int y1, int z1) { p1 = x1, p2 = y1, p3 = z1; }
};
#pragma endregion

#pragma region 函数声明
void readfile();
void init();
void display();
void mouse(int btn, int state, int x, int y);
void keyboard(unsigned char key, int x, int y);
void mainmenu(int id);
void submenu(int id);
void cross(point a, point b, point c, GLfloat *n);//计算叉积
void reshape(int w, int h);
#pragma endregion

#pragma region 变量
bool pickornot = false;//是否拾取
float rotateangle = 10;
bool wireframe = true;//是否为线框
int windowsx = 960;
int windowsy = 640;
#pragma endregion

#pragma region 全局变量
int point_num;//点的数量
int triangle_num;//三角形的数量
vector<point> pointlist;//存储点
vector<triangle> trilist;//存储三角形
#pragma endregion

int main(int argc, char **argv)
{
	//读取文件
	cout << "按下任意键开始读取文件" << endl;
	system("pause");
	readfile();//读取文件
	cout << "读取完毕" << endl;
	//绘制
	cout << "按下任意键开始绘制" << endl;
	system("pause");
	glutInit(&argc, argv);  //对GLUT进行初始化，这个函数必须在其它的GLUT使用之前调用一次
	glutInitDisplayMode(GLUT_SINGLE | GLUT_RGB | GLUT_DEPTH); //窗口显示模式(RGB)
	glEnable(GL_DEPTH_TEST);
	glutInitWindowSize(960, 640);//初始化窗体的大小
	glutInitWindowPosition(0, 0);//显示位置
	glutCreateWindow("Stanford_Bunny");//根据前面设置的信息创建窗口
	init();


	int sub_menu = glutCreateMenu(submenu);//创建submenu菜单
	glutAddMenuEntry("10", 1);//为菜单增加一些条目(菜单名称 当你选择菜单里的一项后，这个值就返回给上面的 glutCreateMenu里调用的函数)
	glutAddMenuEntry("22.5", 2);
	glutAddMenuEntry("45", 3);
	glutAddMenuEntry("90", 4);
	glutCreateMenu(mainmenu);
	glutAddMenuEntry("Pick_Mode", 1);
	glutAddMenuEntry("Wireframe_mode", 2);//线框模式
	glutAddMenuEntry("Full_mode", 3);//填充模式
	glutAddSubMenu("rotateangle=", sub_menu);
	glutAttachMenu(GLUT_MIDDLE_BUTTON);//按下鼠标的中间键弹出菜单
	glutDisplayFunc(display);
	glutMouseFunc(mouse);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);//改变窗口大小时保持图形比例
	glutMainLoop();
	cout << "绘制完毕" << endl;
}

void readfile()//读取文件
{
	//用到的变量
	string filename = "bunny_iH.ply2";	//ply2格式： 点个数 边个数 所有的点 所有的边（注意前面有个3）
	//string filename = "bunny.ply";
	fstream infile;
	string result;
	stringstream ss;//string to int
	//开始读取文件
	infile.open(filename, ios::in);
	//读取文件中点的个数
	getline(infile, result);
	ss << result;
	ss >> point_num;
	ss.clear();
	cout << "点的个数:" << point_num << endl;
	//读取文件中三角形的个数
	getline(infile, result);
	ss << result;
	ss >> triangle_num;
	ss.clear();
	cout << "三角形的个数:" << triangle_num << endl;
	//读取所有的点
	double x, y, z;
	for (int i = 0; i < point_num; i++)
	{
		getline(infile, result);
		ss << result;
		ss >> x;
		ss >> y;
		ss >> z;
		ss.clear();
		point newpoint = point(x, y, z);
		pointlist.push_back(newpoint);
	}
	//读取所有的边
	int t;//不需要的数据，去掉数据中第一列的3
	int p1, p2, p3;
	for (int i = 0; i < triangle_num; i++)
	{
		getline(infile, result);
		ss << result;
		ss >> t;
		ss >> p1;
		ss >> p2;
		ss >> p3;
		ss.clear();
		triangle newtri = triangle(p1, p2, p3);
		trilist.push_back(newtri);
	}
	infile.close();//读取文件结束
}

void init()
{
	glEnable(GL_DEPTH);
	glEnable(GL_DEPTH_TEST); //开启更新深度缓冲区的功能(如果通过比较后深度值发生变化了，会进行更新深度缓冲区的操作)
	glClearColor(1.0, 1.0, 1.0, 0.0);//设置背景颜色为白色
	glClear(GL_COLOR_BUFFER_BIT);//把窗口清除为当前颜色
	glClear(GL_DEPTH_BUFFER_BIT);//清除深度缓冲区


	glMatrixMode(GL_PROJECTION); //将当前矩阵指定为投影矩阵 把物体投影到一个平面上
	glLoadIdentity();//把矩阵设为单位矩阵
	//glOrtho(left, right, bottom, top, near, far)
	//(投影变换函数)创建一个正交平行的视景体，一般用于"物体不会因为离屏幕的远近而产生大小的变换"的情况
	//glOrtho将产生一个矩阵，这个矩阵将填到投影矩阵上
	glOrtho(-13, 13, -13, 13, -100, 100);
	glMatrixMode(GL_MODELVIEW);//模型视图 对模型视景的操作
	/*会采纳每个顶点的颜色，非顶点的部分根据周边顶点的颜色采取平滑过渡的模式，
	对于线段图元，线段上各点的颜色会根据两顶点的颜色通过插值得到*/
	glShadeModel(GL_SMOOTH);//采用Gourand着色
	
	//光照

	/* 光信号light0 */
	GLfloat light_ambient[] = { 0.0, 0.0, 0.0, 1.0 };//RGBA模式下的环境光
	GLfloat light_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };//RGBA模式下的漫反射光
	//GLfloat light_diffuse[] = { 0.157, 0.784, 0.784, 0.0 };//青色
	GLfloat light_specular[] = { 1.0, 1.0, 1.0, 1.0 };//RGBA模式下的镜面反射光

	/* 材料的颜色——材质对光的红、绿、蓝三原色的反射率 */

	GLfloat mat_ambient[] = { 1.0, 1.0, 1.0, 1.0 };//环境光分量
	GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };//漫反射光分量 第四个参数为1.0时，光源为定位光源
	//GLfloat mat_diffuse[] = { 1.0, 0.0, 0.0, 0.0 };//红色
	GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };//镜面反射光分量 全白光
	GLfloat mat_shininess = { 100.0 };//镜面指数 该值越大，表示材质越类似于镜面，光源照射到上面后，产生较小的亮点

	
	/* 设置光信号light0 灯光的三个分量*/
	glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
	/* 定义所有多边形前面(front)的材质属性 */
	
	glMaterialfv(GL_FRONT, GL_AMBIENT, mat_ambient);//材料的环境光颜色
	glMaterialfv(GL_FRONT, GL_DIFFUSE, mat_diffuse);
	glMaterialfv(GL_FRONT, GL_SPECULAR, mat_specular);
	glMaterialf(GL_FRONT, GL_SHININESS, mat_shininess);
	/* 照明 */
	glShadeModel(GL_SMOOTH); //采用Gourand着色
	glEnable(GL_LIGHTING); //启用灯源
	glEnable(GL_LIGHT0);  //启用0号灯
	glEnable(GL_DEPTH_TEST); //开启z buffer
}


void display()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glColor3f(0.0, 0.0, 0.0);
	if (wireframe)
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);//设置为线框模式 所有图形都会变成线
	else
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);//设置为填充模式
	GLfloat n[3];
	
	for (int i = 0; i < triangle_num; i++)
	{
		glLineWidth(2.0f);// 设置线的宽度 
		cross(pointlist[trilist[i].p1], pointlist[trilist[i].p2], pointlist[trilist[i].p3], n);//计算叉积
		glBegin(GL_TRIANGLES);//开始绘制三角形
		glNormal3fv(n);
		glVertex3f(pointlist[trilist[i].p1].x, pointlist[trilist[i].p1].y, pointlist[trilist[i].p1].z);
		glVertex3f(pointlist[trilist[i].p2].x, pointlist[trilist[i].p2].y, pointlist[trilist[i].p2].z);
		glVertex3f(pointlist[trilist[i].p3].x, pointlist[trilist[i].p3].y, pointlist[trilist[i].p3].z);
		glEnd();
	}
	glFlush();
}

void mouse(int btn, int state, int x, int y)
{
	if (btn == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		//cout << x << " " << y << endl;
		if (pickornot == true)
		{
			cout << x << " " << y << endl;
		}
	}
}

void keyboard(unsigned char key, int x, int y)
{
	/*
		平移
	*/ 

	//x移负 glTranslatef
	if (key == 'q')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(1.0, 0, 0);
		glutPostRedisplay();
	}
	//x移正 glTranslatef
	if (key == 'w')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(-1.0, 0, 0);
		glutPostRedisplay();
	}
	//y移负
	if (key == 'e')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0, -1.0, 0);
		glutPostRedisplay();
	}
	//y移正
	if (key == 'r')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0, 1.0, 0);
		glutPostRedisplay();
	}
	//z移负
	if (key == 't')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0, 0, -1.0);
		glutPostRedisplay();
	}
	//z移正
	if (key == 'y')
	{
		glMatrixMode(GL_MODELVIEW);
		glTranslatef(0, 0, 1.0);
		glutPostRedisplay();
	}

	/*
		旋转
	*/

	//x逆 glRotatef
	if (key == 'a')
	{
		glMatrixMode(GL_MODELVIEW);
		//glLoadIdentity();
		glRotatef(-rotateangle, 1.0, 0.0, 0.0);
		glutPostRedisplay();//窗口显示将被回调，从而重新显示窗口的正常面板 使每一次移动都能在窗口中显示
	}
	//x顺
	if (key == 's')
	{
		glMatrixMode(GL_MODELVIEW);
		glRotatef(rotateangle, 1.0, 0.0, 0.0);
		glutPostRedisplay();
	}
	//y逆
	if (key == 'd')
	{
		glMatrixMode(GL_MODELVIEW);
		glRotatef(-rotateangle, 0.0, 1.0, 0.0);
		glutPostRedisplay();

	}
	//y顺
	if (key == 'f')
	{
		glMatrixMode(GL_MODELVIEW);
		glRotatef(rotateangle, 0.0, 1.0, 0.0);
		glutPostRedisplay();
	}
	//z逆
	if (key == 'g')
	{
		glMatrixMode(GL_MODELVIEW);
		glRotatef(-rotateangle, 0.0, 0.0, 1.0);
		glutPostRedisplay();
	}
	//z顺
	if (key == 'h')
	{
		glMatrixMode(GL_MODELVIEW);
		glRotatef(rotateangle, 0.0, 0.0, 1.0);
		glutPostRedisplay();
	}

	/*
		缩放
	*/

	//恢复 glLoadIdentity
	if (key == 'p')
	{
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glutPostRedisplay();
	}
	//缩小 glScalef
	if (key == 'z')
	{
		glMatrixMode(GL_MODELVIEW);
		glScalef(0.95f, 0.95f, 0.95f);
		glutPostRedisplay();
	}
	//放大
	if (key == 'x')
	{
		glMatrixMode(GL_MODELVIEW);
		glScalef(1.05f, 1.05f, 1.05f);
		glutPostRedisplay();
	}

}

void mainmenu(int id)
{
	switch (id)
	{
		case 1:
		{
			pickornot = true;
			wireframe = true;
			glutPostRedisplay();
		}
		break;
		case 2:
		{
			pickornot = false;
			wireframe = true;
			glutPostRedisplay();
		}
		break;
		case 3:
		{
			pickornot = false;
			wireframe = false;
			glutPostRedisplay();
		}
		break;
	}
}

void submenu(int id)
{
	switch (id)
	{
		case 1:rotateangle = 10; break;
		case 2:rotateangle = 22.5; break;
		case 3:rotateangle = 45; break;
		case 4:rotateangle = 90; break;
	}
}

void reshape(int w, int h)//改变窗口大小时保持图形比例
{
	glViewport(0, 0, w, h);
	cout << w << " " << h << endl;//改变窗口大小时输出窗口大小
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-13*(w/960.0), 13 * (w/960.0), -13 * (h/640.0), 13 * (h/640.0 ), -100, 100);//产生新的正交平行视景体
	glMatrixMode(GL_MODELVIEW);
}

void normalize(GLfloat *p)//归一化
{
	double d = 0.0;
	for (int i = 0; i < 3; i++)
		d += p[i] * p[i];
	d = sqrt(d);
	if (d > 0.0)
	{
		for (int i = 0; i < 3; i++)
			p[i] /= d;
	}
}

void cross(point a, point b, point c, GLfloat *n)//计算叉积 绘制三角形时使用
{
	n[0] = (b.y - a.y)*(c.z - a.z) - (b.z - a.z)*(c.y - a.y);
	n[1] = (b.z - a.z)*(c.x - a.x) - (b.x - a.x)*(c.z - a.z);
	n[2] = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
	normalize(n);//变为单位向量
}



