#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#define BOARD_SIZE 12
#define EMPTY 0
#define BLACK 1
#define WHITE 2
clock_t start1, finish;
double duration;
//����С������� 
#ifndef SIX
#define DEPTH 3 //���

#endif
#ifdef SIX
#define DEPTH 6 

#endif
#define KILLDEPTH 14 //��ɱ���
#define NINF -9223372036854775807
#define PINF 9223372036854775807

typedef int BOOL;
typedef long long LL;
typedef struct Info {
    int more;//����
    int win5;//20000000
    int alive4;//��4
    int dalive4;//��4 10000
    int dead4;//��4 5000
    int alive3;//��3 10000
    int dalive3;//��3 1000
    int dead3;//��3 500
    int alive2; //��2 1000
    int dalive2; //��2 200
    int dead2; //��2 50
    int alive1; //��1 100
    int dalive1; //��1 10
    int dead1; //��1 5
}Info;
#define TRUE 1
#define FALSE 0
#define START "START"
#define PLACE "PLACE"
#define TURN "TURN"
#define END "END"
struct Point
{
    int x;
    int y;
};
struct Move {
    Point p;
    int score;
};

char board[BOARD_SIZE][BOARD_SIZE] = { 0 };
int myFlag;
int enemyFlag;
int num = 0;
//score.c
Point nextPoint(Point p, int d, int di);
void getBoundary(Point p, int d, int* s, int way, int player);
int getLength(Point p, int d, int* left, int* right, int player);
int singleScore(Point p, int player);
LL wholeScore(int player);
Info typeAnalysis(int length, int* left, int* right, int player);
Info getInfo(Point p, int player);
int partition(Move* s, int low, int high);
void quickSort(Move* s, int low, int high);
void quickSortMoves(Move* s, int len);
//minmax.c
LL minmax(int depth, LL alpha, LL beta, int player);
LL alphaBeta(int depth, LL alpha, LL beta, int player, Point p);
int inspireFind(Move* s, int player);
int killSearch(void);
int maxKill(int depth, Point p);
int minKill(int depth, Point p);
int findComKill(Move* move);
int findHumKill(Move* move);
void debug(const char* str)
{
    printf("DEBUG %s\n", str);
    fflush(stdout);
}
BOOL isInBoard(int x, int y)
{
    return x >= 0 && x < BOARD_SIZE && y >= 0 && y < BOARD_SIZE;
}
//�ж�����xy��Χ�Ƿ�������
int JudgeFive(int x, int y)
{
    int i, j, k;
    const int direction[4][2] = { {1,0},{0,1},{1,1},{1,-1} };//��ʾ������б�ĸ�����
    for (i = 0; i < 4; ++i)
    {
        const int d[2] = { -1,1 };//��ʾ������������
        int count = 1;
        for (j = 0; j < 2; ++j) {
            for (k = 1; k <= 4; ++k) {
                int row = x + k * d[j] * direction[i][0];//�к�
                int col = y + k * d[j] * direction[i][1];//�к�
                if (row >= 0 && row < BOARD_SIZE && col >= 0 && col < BOARD_SIZE &&
                    board[x][y] == board[row][col])
                    count++;
                else
                    break;
            }
        }
        if (count >= 5)
            return 1;//����������
    }
    return 0;
}

void place(struct Point p)
{
    board[p.x][p.y] = enemyFlag;
}
void initAI(int me)
{
    enemyFlag = 3 - me;
}


//����p��d����������di����ĵ�
struct Point nextPoint(struct Point p, int d, int di) {
    const int direction[4][2] = { {1,0},{0,1},{1,1},{1,-1} };
    struct Point np;
    np.x = p.x + di * direction[d][0];
    np.y = p.y + di * direction[d][1];
    return np;
}
//��ȡ����id
int opp(int player) {
    return (player == 1) ? 2 : 1;
}

//���Ӻ���
void set(struct Point p, int player) {
    board[p.x][p.y] = player;
}
//ȡ������
void unSet(Point p) {
    board[p.x][p.y] = 0;
}
//��ȡp��d�����������Ϣ
void getBoundary(Point p, int d, int* s, int way, int player) {
    for (int i = 0; i < 4; i++)
    {
        struct Point np = nextPoint(p, d, way * i);
        if (isInBoard(np.x, np.y))//��������
            s[i] = board[np.x][np.y];
        else //�����߽� ��Ϊ�Է���
            s[i] = opp(player);
    }
}
//��ȡp�����ӵĳ��Ⱥ���������4�ӵ���Ϣ
int getLength(Point p, int d, int* left, int* right, int player)
{
    int i;
    int count = 1;
    for (i = 1; ; i++) {//����ж�
        Point np = nextPoint(p, d, -i);
        if (isInBoard(np.x, np.y) && player == board[np.x][np.y])
            count++;
        else {
            getBoundary(np, d, left, -1, player);
            break;
        }
    }
    for (i = 1; ; i++) {//�ұ��ж�
        Point np = nextPoint(p, d, i);
        if (isInBoard(np.x, np.y) && player == board[np.x][np.y])
            count++;
        else {
            getBoundary(np, d, right, 1, player);
            break;
        }
    }
    return count;
}
//����������Ŀ�ͱ�Ե��Ϣ�ж�����
Info typeAnalysis(int length, int* left, int* right, int player)
{
    Info temp = { 0 };
    if (length > 5)
        temp.more++;
    else if (length == 5)
        temp.win5++;
    else if (length == 4)
    {
        if (left[0] == 0 && right[0] == 0)
            temp.alive4++;
        else if (left[0] == 0 || right[0] == 0)
            temp.dalive4++;
        else
            temp.dead4++;
    }
    else if (length == 3)
    {
        if ((left[0] == 0 && left[1] == player) && (right[0] == 0 && right[1] == player))
            temp.dalive4 += 2;
        else if ((left[0] == 0 && left[1] == player) || (right[0] == 0 && right[1] == player))
            temp.dalive4++;
        else if (left[0] == 0 && right[0] == 0 && (left[1] == 0 || right[1] == 0))
            temp.alive3++;
        else if ((left[0] == 0 && left[1] == 0) || (right[0] == 0 && right[1] == 0))
            temp.dalive3++;
        else
            temp.dead3++;
    }
    else if (length == 2)
    {
        if ((left[0] == 0 && left[1] == player && left[2] == player) &&
            (right[0] == 0 && right[1] == player && right[2] == player))
            temp.dalive4 += 2;
        else if ((left[0] == 0 && left[1] == player && left[2] == player) ||
            (right[0] == 0 && right[1] == player && right[2] == player))
            temp.dalive4++;
        else if (left[0] == 0 && right[0] == 0 &&
            ((left[1] == player && left[2] == 0) ||
                (right[1] == player && right[2] == 0)))
            temp.alive3++;
        else if ((left[0] == 0 && left[2] == 0 && left[1] == player) ||
            (right[0] == 0 && right[2] == 0 && right[1] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && right[0] == 0) &&
            (left[1] == player || right[1] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && left[1] == 0 && left[2] == player) ||
            (right[0] == 0 && right[1] == 0 && right[2] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && right[0] == 0 && right[1] == 0 && right[2] == 0) ||
            (left[0] == 0 && left[1] == 0 && right[0] == 0 && right[1] == 0) ||
            (left[0] == 0 && left[1] == 0 && left[2] == 0 && right[0] == 0))
            temp.alive2++;
        else if ((left[0] == 0 && left[1] == 0 && left[2] == 0) ||
            (right[0] == 0 && right[1] == 0 && right[2] == 0))
            temp.dalive2++;
    }
    else if (length == 1) {
        if ((left[0] == 0 && left[1] == player && left[2] == player && left[3] == player) &&
            (right[0] == 0 && right[1] == player && right[2] == player && right[3] == player))
            temp.dalive4 += 2;
        else if ((left[0] == 0 && left[1] == player && left[2] == player && left[3] == player) ||
            (right[0] == 0 && right[1] == player && right[2] == player && right[3] == player))
            temp.dalive4++;
        else if ((left[0] == 0 && right[0] == 0) && ((left[1] == player && left[2] == player && left[3] == 0) ||
            (right[1] == player && right[2] == player && right[3] == 0)))
            temp.alive3++;
        else if ((left[0] == 0 && right[0] == 0) &&
            ((left[1] == player && left[2] == player) || (right[1] == player && right[2] == player)))
            temp.dalive3++;
        else if ((left[0] == 0 && left[3] == 0 && left[1] == player && left[2] == player) ||
            (right[0] == 0 && right[3] == 0 && right[1] == player && right[2] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && left[1] == 0 && left[2] == player && left[3] == player) ||
            (right[0] == 0 && right[1] == 0 && right[2] == player && right[3] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && left[2] == 0 && left[1] == player && left[3] == player) ||
            (right[0] == 0 && right[2] == 0 && right[1] == player && right[3] == player))
            temp.dalive3++;
        else if ((left[0] == 0 && right[0] == 0 && right[2] == 0 && right[1] == player)
            && (left[1] == 0 || right[3] == 0))
            temp.alive2++;
        else if ((right[0] == 0 && left[0] == 0 && left[2] == 0 && left[1] == player) &&
            (right[1] == 0 || left[3] == 0))
            temp.alive2++;
        else if ((left[0] == 0 && right[0] == 0 && right[1] == 0 && right[3] == 0 && right[2] == player) ||
            (right[0] == 0 && left[0] == 0 && left[1] == 0 && left[3] == 0 && left[2] == player))
            temp.alive2++;
        else if ((left[0] == 0 && left[2] == 0 && left[3] == 0 && left[1] == player) ||
            (right[0] == 0 && right[2] == 0 && right[3] == 0 && right[1] == player))
            temp.dalive2++;
        else if ((left[0] == 0 && right[0] == 0 && right[1] == 0 && left[1] == player) ||
            (right[0] == 0 && left[0] == 0 && left[1] == 0 && right[1] == player))
            temp.dalive2++;
        else if ((left[0] == 0 && left[1] == 0 && left[3] == 0 && left[2] == player) ||
            (right[0] == 0 && right[1] == 0 && right[3] == 0 && right[2] == player))
            temp.dalive2++;
        else if ((left[0] == 0 && left[1] == 0 && right[0] == 0 && left[2] == player) ||
            (right[0] == 0 && right[1] == 0 && left[0] == 0 && right[2] == player))
            temp.dalive2++;
        else if ((left[0] == 0 && left[1] == 0 && left[2] == 0 && left[3] == player) ||
            (right[0] == 0 && right[1] == 0 && right[2] == 0 && right[3] == player))
            temp.dalive2++;
        else if (left[0] == 0 && right[0] == 0)
            temp.alive1++;
        else if (left[0] == 0 || right[0] == 0)
            temp.dalive1++;
        else
            temp.dead1++;
    }
    return temp;
}
//��ȡp����Χ������
Info getInfo(struct Point p, int player) {
    Info temp = { 0 };
    for (int i = 0; i < 4; i++) {
        Info info[4];
        int length;
        int left[4], right[4];
        length = getLength(p, i, left, right, player);
        info[i] = typeAnalysis(length, left, right, player);
        temp.more += info[i].more;
        temp.win5 += info[i].win5;
        temp.alive4 += info[i].alive4;
        temp.dalive4 += info[i].dalive4;
        temp.dead4 += info[i].dead4;
        temp.alive3 += info[i].alive3;
        temp.dalive3 += info[i].dalive3;
        temp.dead3 += info[i].dead3;
        temp.alive2 += info[i].alive2;
        temp.dalive2 += info[i].dalive2;
        temp.dead2 += info[i].dead2;
        temp.alive1 += info[i].alive1;
        temp.dalive1 += info[i].dalive1;
        temp.dead1 += info[i].dead1;
    }
    return temp;
}
//������������
LL wholeScore(int player) {
    LL comScore = 0, humScore = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            struct Point p = { i,j };
            if (board[i][j] == player)//�������ӵĵ�������
                comScore += singleScore(p, player);
            else if (board[i][j] == opp(player))//�Է����ӵĵ�������
                humScore += singleScore(p, opp(player));
        }
    }
    return comScore - humScore;//�����ּܷ��Է��ܷ� �õ���ǰ�Լ�����˵�ľ��Ʒ�
}int singleScore(struct Point p, int player) {
    int score = 0;
    Info info = getInfo(p, player);
    score += ((info.alive4 << 16) + (info.dalive4 << 13) + (info.dead4 << 11) +
        (info.alive3 << 14) + (info.dalive3 << 10) + (info.dead3 << 6) +
        (info.alive2 << 10) + (info.dalive2 << 7) + (info.dead2 << 3) +
        (info.alive1 << 7) + (info.dalive1 << 3) + info.dead1);


    if (info.more >= 1 || info.win5 >= 1)
        score += 20000000;
    if (info.alive4 >= 1 || info.dalive4 >= 2 || (info.dalive4 >= 1 && info.alive3 >= 1) || info.alive3 >= 2)//��ʤ
        score += 10000000;

    return score;
}


//����������������
int ai_x, ai_y;
//�жϵ��Ƿ����ھ� ����Ч��
int hasNeighbor(Point p) {

    //�������������ӵ�6��������
    for (int i = 0; i < 4; i++) {
        for (int j = -6; j <= 6; j++) {//���Ҿ���3
            if (j != 0) {
                Point np = nextPoint(p, i, j);
                if (isInBoard(np.x, np.y) && board[np.x][np.y] != 0)
                    return 1;
            }
        }
    }

    return 0;//û���ھ�
}
//����С�����������
LL minmax(int depth, LL alpha, LL beta, int player) {
    Move moves[224];
    int length = inspireFind(moves, player);//���������ӵ�
    

    for (int i = 0; i < length; i++) {
        set(moves[i].p, player);//ģ������
        LL temp = -alphaBeta(depth - 1, -beta, -alpha, opp(player), moves[i].p);//�ݹ�
        unSet(moves[i].p);//��ԭ����
        if (temp >= beta) {
            return beta;//��֦
        }
        if (temp > alpha) {
            alpha = temp;
            ai_x = moves[i].p.x;//��¼�������
            ai_y = moves[i].p.y;
        }
    }
    return alpha;
}
//������Сֵ���� ��һ���ۺ���˫���ļ���д��
LL alphaBeta(int depth, LL alpha, LL beta, int player, Point p) {
    if (JudgeFive(p.x, p.y))//���Ӻ����
        return wholeScore(player);//���ص�ǰ�����
    if (depth == 0) {//���Ϊ0
        return wholeScore(player);
    }


    Move moves[224];
    int length = inspireFind(moves, player);
    
    

    for (int i = 0; i < length; i++) {
        set(moves[i].p, player);//ģ������
        LL temp = -alphaBeta(depth - 1, -beta, -alpha, opp(player), moves[i].p);//ע��Ҫȡ��ֵ������alpha��beta
        unSet(moves[i].p);//��ԭ����
        if (temp >= beta) {
            return beta;//��֦
        }
        if (temp > alpha) {
            alpha = temp;
        }
    }
    return alpha;
}
//����ʽ����
int inspireFind(Move* scoreBoard, int player) {
    int length = 0;
    for (int i = 0; i < BOARD_SIZE; i++) {
        for (int j = 0; j < BOARD_SIZE; j++) {
            if (board[i][j] == 0) {//ֻ�������ϵĿ�λ
                Point p = { i, j };

                if (hasNeighbor(p)) {
                    scoreBoard[length].score = singleScore(p, player);//�ҷ��ʺϽ����ĵ�
                    scoreBoard[length].score += singleScore(p, opp(player));//�Է���Ҫ���صĵ�
                    scoreBoard[length++].p = p;
                }
            }
        }
    }
    quickSortMoves(scoreBoard, length);//���ݷ����������� �������Ӽ�֦Ч��
    return length;
}
// ��������ķ�������
int partition(Move* s, int low, int high) {
    int pivot = s[high].score;
    int i = (low - 1);

    for (int j = low; j <= high - 1; j++) {
        if (s[j].score >= pivot) {
            i++;
            // ���� s[i] �� s[j]
            Move temp = s[i];
            s[i] = s[j];
            s[j] = temp;
        }
    }

    // ���� s[i + 1] �� s[high]
    Move temp = s[i + 1];
    s[i + 1] = s[high];
    s[high] = temp;

    return (i + 1);
}

// ����������
void quickSort(Move* s, int low, int high) {
    if (low < high) {
        int pi = partition(s, low, high);

        // �ֱ��������������п�������
        quickSort(s, low, pi - 1);
        quickSort(s, pi + 1, high);
    }
}

// ���ÿ�������
void quickSortMoves(Move* s, int len) {
    quickSort(s, 0, len - 1);
}


void start(int flag)
{
    //When FIELD is 1, black stones, FIELD is 2,  plays with white stones
    memset(board, 0, sizeof(board));
    int middlePlace = BOARD_SIZE / 2;//6
    board[middlePlace - 1][middlePlace - 1] = WHITE;//5,5
    board[middlePlace][middlePlace] = WHITE;//6,6
    board[middlePlace - 1][middlePlace] = BLACK;//5,6
    board[middlePlace][middlePlace - 1] = BLACK;//6,5
    initAI(flag);
}
void turn()
{
    start1 = clock();
    
    minmax(DEPTH, NINF, PINF, myFlag);
    Point p = { ai_x,ai_y };
    finish = clock();

    duration = (double)(finish - start1) / CLOCKS_PER_SEC;
    /*printf("%d %d %f", ai_x, ai_y, duration);*/
   printf("%d %d", ai_x, ai_y); 
    printf("\n");
    fflush(stdout);
    set(p, myFlag);
    num++;
    return;
}
void end(int x)
{
    exit(0);
}

void loop()
{
    char tag[10] = { 0 };
    struct Point point;
    point.x = 0;
    point.y = 0;

    int status;
    while (TRUE)
    {
        memset(tag, 0, sizeof(tag));
        scanf("%s", tag);
        if (strcmp(tag, START) == 0)
        {
            scanf("%d", &myFlag);
            start(myFlag);
            printf("OK\n");
            fflush(stdout);
        }
        else if (strcmp(tag, PLACE) == 0)
        {
            scanf("%d %d", &point.x, &point.y);
            place(point);
        }
        else if (strcmp(tag, TURN) == 0)
        {
            turn();
        }
        else if (strcmp(tag, END) == 0)
        {
            scanf("%d", &status);
            end(status);
        }
    }
}
int main(int argc, char* argv[])
{

    loop();
    return 0;
}