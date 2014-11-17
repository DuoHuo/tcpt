package com.nuist.tcptlock;

import com.nuist.tcptlock.LockScreenService;
import com.nuist.tcptlock.MainActivity;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.Bundle;
import android.os.Handler;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.KeyEvent;
import android.view.View.OnKeyListener;
import android.view.Window;
import android.view.WindowManager;
import android.view.inputmethod.InputMethodManager;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.ImageView;

public class MainActivity extends Activity implements OnClickListener {

	private EditText password;
	private ImageButton setting;
	private Intent intentService;
	public static final int LOCKED_SUCCESS = 1;
	public static final int VIEW_INVALIDATE = 2;
	public boolean setting_flag = false;

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		// 取消标题栏
		requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_DISMISS_KEYGUARD);
		getWindow().addFlags(WindowManager.LayoutParams.FLAG_SHOW_WHEN_LOCKED);
		// 全屏显示
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);

		setContentView(R.layout.lock);
		initView();
		startService();

	}

	private void initView() {
		password = (EditText) findViewById(R.id.passwordId);
		//password.setOnClickListener(this);
		setting = (ImageButton) findViewById(R.id.settingId);
		setting.setOnClickListener(this);
		password.setOnKeyListener(new OnKeyListener() {
			public boolean onKey(View v, int keyCode, KeyEvent event) {
				if (KeyEvent.KEYCODE_ENTER == keyCode
						&& event.getAction() == KeyEvent.ACTION_DOWN) {
					validate();
					return true;
				}
				return false;
			}
		});
	}

	/* 验证密码并结束activity回到主屏 */
	private void validate() {
		System.out.println(password.getText().toString());
		if (TcptJni.auth(password.getText().toString())==1) {
			if (setting_flag == false) {
				MainActivity.this.finish();
			} else {
				Intent intent = new Intent(MainActivity.this,
						RegisterActivity.class);
				startActivity(intent);
				setting_flag = false;
			}
		} else {
			// 验证失败

		}
	}

	/**
	 * 启动服务
	 */
	private void startService() {
		// TODO Auto-generated method stub
		intentService = new Intent(MainActivity.this, LockScreenService.class);
		startService(intentService);
	}

	/**
	 * 
	 * 屏蔽掉Home键
	 * 
	 */
	@Override
	public void onAttachedToWindow() {
		// TODO Auto-generated method stub
		// getWindow().setType(WindowManager.LayoutParams.TYPE_KEYGUARD_DIALOG);
		super.onAttachedToWindow();
	}

	/**
	 * 
	 * 屏蔽掉返回键
	 * 
	 */
	@Override
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		// TODO Auto-generated method stub

		if (keyCode == KeyEvent.KEYCODE_BACK) {
			return true;
		} else {
			return super.onKeyDown(keyCode, event);
		}

	}

	/*
	 * @Override
	 * 
	 * public void onStart() { super.onStart();
	 * 
	 * installedReceiver = new MyInstalledReceiver(); IntentFilter filter = new
	 * IntentFilter();
	 * 
	 * filter.addAction("android.intent.action.PACKAGE_ADDED");
	 * filter.addAction("android.intent.action.PACKAGE_REMOVED");
	 * filter.addDataScheme("package");
	 * 
	 * this.registerReceiver(installedReceiver, filter); }
	 */

	@Override
	public void onClick(View v) {
		// TODO Auto-generated method stub
		switch (v.getId()) {
		case R.id.passwordId:
			// 密码框响应
			// validate();
			break;
		case R.id.settingId:
			setting_flag = true;
		}
	}
}
