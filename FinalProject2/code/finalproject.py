from aip import AipOcr
import glob

""" 你的 APPID AK SK """
APP_ID = '18078916'
API_KEY = 'GnKmz131THPpodZgzUHNKK6n'
SECRET_KEY = 'G5LuCE8rqojqDf2S588sqc7FjGIDxaDz'

client = AipOcr(APP_ID, API_KEY, SECRET_KEY)

""" 读取图片 """
def get_file_content(filePath):
    with open(filePath, 'rb') as fp:
        return fp.read()

options = {}
options["language_type"] = "CHN_ENG"
options["detect_direction"] = "true"
options["detect_language"] = "false"
options["probability"] = "false"

img_dir = r"D:\TH\CV\Dataset"

with open("result.txt", 'w') as f:
	for src in glob.glob(img_dir + "/*.jpg"):
		print(src)
		image = get_file_content(src)
		result = client.basicGeneral(image, options)
		result = result['words_result']
		print(result)
		f.write(src + "\n")
		for i in result:
			for k, v in i.items():
				f.write(v + "\n")
		f.write("\n")	
