package com.nuist.tcptlock;

import java.io.DataOutputStream;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;

public class RegisterActivity extends Activity implements OnClickListener {

	private EditText setPassword;
	private EditText setCataglogue;
	private Button confirmPassword;
	private Button confirmCatalogue;
	private Button openSystem;
	private Button closeSystem;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// 取消标题栏
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		// 全屏
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.set);
		initView();
	}

	private void initView() {

		setPassword = (EditText) findViewById(R.id.passwordEditTextId);
		setCataglogue = (EditText) findViewById(R.id.CatalogueEditTextId);
		
		confirmPassword = (Button) findViewById(R.id.confirmPasswordButtonId);
		confirmPassword.setOnClickListener(this);

		confirmCatalogue = (Button) findViewById(R.id.confirmCatalogueButtonId);
		confirmCatalogue.setOnClickListener(this);
		
		openSystem = (Button) findViewById(R.id.openSyetemId);
		openSystem.setOnClickListener(this);
		
		closeSystem = (Button) findViewById(R.id.closeSystemId);
		closeSystem.setOnClickListener(this);
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.confirmPasswordButtonId:
			//确认密码按钮响应
            TcptJni.reg(setPassword.getText().toString());
			break;
		case R.id.confirmCatalogueButtonId:
			//确认目录按钮响应
			String cmd = "echo " + setCataglogue.getText().toString() + " >> /sdcard/tcpt/mount-list\n";
			Process process = null;
			try {
				process = Runtime.getRuntime().exec("su");
				// 这里是主要程序代码ATAAW.COM
				DataOutputStream os = new DataOutputStream(process
						.getOutputStream());
				os.writeBytes(cmd);
				os.writeBytes("exit\n");
				os.flush();
				process.waitFor();
			} catch (Exception e) {
				e.printStackTrace();
			} finally {
				process.destroy();
			}
			break;
		case R.id.openSyetemId:
			//开启系统按钮响应
			break;
		case R.id.closeSystemId:
			//关闭系统按钮响应
			  TcptJni.clearkey();
			break;	
			
		}
	}

}
