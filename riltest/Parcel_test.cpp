#include <binder/Parcel.h>
#include <stdio.h>
#include <stdlib.h>

int main()
{
    using namespace android;
    status_t status;
    size_t parcel_position;
    int intp=987654;
    char charp='g';
    float floatp=3.14159;
    double doublep=12345.6789012;
    long longp=1234567890;
    char *stringp="this is my parcel test!";

    Parcel p;
    parcel_position = p.dataPosition();
    printf(" Local parcel_position:%d\n",parcel_position);
    /*************写int类型***************/
    status=p.writeInt32(intp);
    if (status==NO_ERROR)
        printf("write int type success!\n");
    else
        printf("write int type fail,the errno=%d\n",errno);
     printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************写char类型***************/
    status=p.writeInt32(charp);
    if (status==NO_ERROR)
        printf("write char type success!\n");
    else
        printf("write char type fail,the errno=%d\n",errno);
    printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************写Float类型***************/
    status=p.writeFloat(floatp);
    if (status==NO_ERROR)
        printf("write Float type success!\n");
    else
        printf("write Float type fail,the errno=%d\n",errno);
    printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************写Double类型***************/
    status=p.writeDouble(doublep);
    if (status==NO_ERROR)
        printf("write Double type success!\n");
    else
        printf("write Double type fail,the errno=%d\n",errno);
    printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************写long类型***************/
    status=p.writeInt64(longp);
    if (status==NO_ERROR)
        printf("write long type success!\n");
    else
        printf("write long type fail,the errno=%d\n",errno);
    printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************写String类型***************/
    status=p.writeCString(stringp);
    if (status==NO_ERROR)
        printf("write String type success!\n");
    else
        printf("write String type fail,the errno=%d\n",errno);
    printf(" Local parcel_position:%d\n",p.dataPosition());
    /*************将parcel读写位置置回原位***************/
    p.setDataPosition(parcel_position);
    /*************读出变量***************/
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read int :%d\n",p.readInt32());
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read char:%c\n",(char)p.readInt32());
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read Float:%f\n",(float)p.readFloat());
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read Double:%f\n",(double)p.readDouble());
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read long:%ld\n",(long)p.readInt64());
    printf(" Local parcel_position:%d\n",p.dataPosition());
    printf("Read String:%s\n",p.readCString());
    printf(" Local parcel_position:%d\n",p.dataPosition());
} 
/********************** 以上是 Parcel_test.cpp 程序 ****************************/
