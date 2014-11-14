package com.nuist.tcptlock;

import android.app.Activity;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.view.WindowManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;

public class MainActivity extends Activity implements OnClickListener {

	private MyInstalledReceiver installedReceiver = null;
	private EditText password;
	private ImageButton setting;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// 取消标题栏
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		// 全屏
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.lock);
		initView();
	}

	private void initView(){
		
		password =(EditText) findViewById(R.id.passwordId);
		password.setOnClickListener(this);
		
		setting = (ImageButton)findViewById(R.id.settingId);
		setting.setOnClickListener(this);
	}
	
	@Override
	public void onStart() {
		super.onStart();

		installedReceiver = new MyInstalledReceiver();
		IntentFilter filter = new IntentFilter();

		filter.addAction("android.intent.action.PACKAGE_ADDED");
		filter.addAction("android.intent.action.PACKAGE_REMOVED");
		filter.addDataScheme("package");

		this.registerReceiver(installedReceiver, filter);
	}

	@Override
	public void onDestroy() {
		if (installedReceiver != null) {
			this.unregisterReceiver(installedReceiver);
		}

		super.onDestroy();
	}

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.passwordId:
			//密码框响应
			break;
		case R.id.settingId:
			Intent intent = new Intent(MainActivity.this,RegisterActivity.class);
			startActivity(intent);
			break;

		}
	}
}
