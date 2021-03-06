package common;

import java.io.BufferedWriter;
import java.io.File;
import java.io.FileOutputStream;
import java.io.OutputStreamWriter;
import java.text.SimpleDateFormat;
import java.util.Date;

public class Log {

	private String log_file_name;
	private BufferedWriter bw;
	
	
	public Log(String filename) throws Exception
	{
		log_file_name = filename;

		File file = new File("log_file_name");
		FileOutputStream fos = new FileOutputStream(file);
		OutputStreamWriter osw = new OutputStreamWriter(fos, "UTF-8");
		bw = new BufferedWriter(osw);

		
	}
	public void write(String msg, TYPE type) 
	{
		SimpleDateFormat df = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");//设置日期格式
		String current_time = df.format(new Date());// new Date()为获取当前系统时间
		
		try{
			
			switch (type) {
			case INFO:
				bw.write("[INFO] " + current_time + " : " +msg + "\n");
				break;
			case WARN:
				bw.write("[WARN] " + current_time + " : " + msg + "\n");
				break;
			case ERROR:
				bw.write("[ERROR] " + current_time + " : " + msg + "\n");
				break;
			}
			bw.flush();
			
		}catch(Exception e)
		{
			e.printStackTrace();
		}

	}
	public static void main(String[] args) {
		// TODO Auto-generated method stub
			
	}

}
