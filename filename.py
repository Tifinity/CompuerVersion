import os
#对所有文件以数字递增的方式重命名
def file_rename():
    i =0
    #需要重命名的文件绝对路径
    path = r"D:\TH\CV\名片-30"
     #读取该文件夹下所有的文件
    filelist = os.listdir(path)  
    #遍历所有文件
    for files in filelist:   
        Olddir = os.path.join(path, files)    #原来的文件路径
        #os.path.splitext(path)  #分割路径，返回路径名和文件扩展名的元组
        #文件名，此处没用到
        print(Olddir)
        filename = os.path.splitext(files)[0]     
        #文件扩展名
        filetype = os.path.splitext(files)[1] 
        Newdir = os.path.join(path, "IMG_" + str(i)+filetype)  
        print(Newdir)
        os.rename(Olddir, Newdir) 
        i = i + 1  
    return True

if __name__ == '__main__':
    file_rename()