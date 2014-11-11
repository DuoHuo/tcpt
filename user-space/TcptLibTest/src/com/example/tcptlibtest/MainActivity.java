package com.example.tcptlibtest;

import java.io.*;
import android.os.Bundle;
import android.app.Activity;
import android.view.Menu;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;

public class MainActivity extends Activity {

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main);

		final EditText editText1 = (EditText) findViewById(R.id.editText1);
		Button button1 = (Button) findViewById(R.id.button1);
		button1.setOnClickListener(new OnClickListener() {
			String s;
			int ret;

			@Override
			public void onClick(View arg0) {

				s = editText1.getText().toString();
				System.out.println(s);
				ret = reg(s);
				if (-1 == ret)
					System.out.println("reg error");
				else if (1 == ret)
					System.out.println("reg ok");
				else
					System.out.println("reg Unknown");
			}
		});
		final EditText editText2 = (EditText) findViewById(R.id.editText2);
		Button button2 = (Button) findViewById(R.id.button2);
		button2.setOnClickListener(new OnClickListener() {
			String s;

			@Override
			public void onClick(View arg0) {
				s = editText2.getText().toString();
				System.out.println(s);

				if (-1 == auth(s))
					System.out.println("Auth error");
				else if (1 == auth(s))
					System.out.println("Auth ok");
				else
					System.out.println("Auth unknown");
			}
		});

		Button button3 = (Button) findViewById(R.id.button3);
		button3.setOnClickListener(new OnClickListener() {
			int ret;

			@Override
			public void onClick(View arg0) {
				Process process = null;
				try {
					process = Runtime.getRuntime().exec("su");
					// 这里是主要程序代码ATAAW.COM
					DataOutputStream os = new DataOutputStream(process
							.getOutputStream());
					os.writeBytes("sh /sdcard/tcpt/insfs.sh\n");
					os.writeBytes("exit\n");
					os.writeBytes("exit\n");
					os.flush();
					process.waitFor();
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					process.destroy();
				}
			}
		});
	}

	public native int reg(String pwd);

	public native int auth(String pwd);

	static {
		System.loadLibrary("testJni");
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

}
