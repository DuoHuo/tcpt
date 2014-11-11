package cn.edu.nuist.wcw.lockscreen;

import android.annotation.SuppressLint;
import android.annotation.TargetApi;
import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Build;
import android.os.Handler;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;
import android.widget.CompoundButton;
import android.widget.CompoundButton.OnCheckedChangeListener;
import android.widget.LinearLayout;
import android.widget.Switch;
import android.widget.TextView;
import android.widget.Toast;

@TargetApi(Build.VERSION_CODES.HONEYCOMB)
public class MyLinearLayout extends LinearLayout {
	private TextView pass; 
	private Button but0, but1, but2, but3, but4, but5, but6, but7, but8, but9,
			butClear, butDel;
	private Switch mSwitch;

	private Activity activity;
	private Handler myHandler;

	private Context context;

	private boolean flag;

	public MyLinearLayout(Context context) {
		super(context);
		this.context = context;
	}

	public MyLinearLayout(Context context, AttributeSet attrs) {
		super(context, attrs);
		this.context = context;
	}

	public MyLinearLayout(Context context, AttributeSet attrs, int defStyle) {
		super(context, attrs, defStyle);
		this.context = context;
	}

	/* When unlocked && flag = true */
	public void doTrue() { //TODO
		Toast.makeText(activity, "flag = true", Toast.LENGTH_SHORT).show();
		Intent intent = new Intent(context,ConfigActivity.class);
                context.startActivity(intent);
                finish();
	}

	/* When unlocked && flag = false */
	public void doFalse() {
		Toast.makeText(activity, "flag = false", Toast.LENGTH_SHORT).show();
	}

	public void setParams(Activity activity, Handler handler) {
		this.activity = activity;
		this.myHandler = handler;
	}

	public void start() {
		this.initViews();
		this.initValues();
		this.setListeners();
	}

	private void initViews() {
		this.pass = (TextView) this.activity.findViewById(R.id.pass);
		this.but0 = (Button) this.activity.findViewById(R.id.but0);
		this.but1 = (Button) this.activity.findViewById(R.id.but1);
		this.but2 = (Button) this.activity.findViewById(R.id.but2);
		this.but3 = (Button) this.activity.findViewById(R.id.but3);
		this.but4 = (Button) this.activity.findViewById(R.id.but4);
		this.but5 = (Button) this.activity.findViewById(R.id.but5);
		this.but6 = (Button) this.activity.findViewById(R.id.but6);
		this.but7 = (Button) this.activity.findViewById(R.id.but7);
		this.but8 = (Button) this.activity.findViewById(R.id.but8);
		this.but9 = (Button) this.activity.findViewById(R.id.but9);
		this.butDel = (Button) this.activity.findViewById(R.id.butDel);
		this.butClear = (Button) this.activity.findViewById(R.id.butClear);
		this.mSwitch = (Switch) this.activity.findViewById(R.id.opener);
	}

	@SuppressLint("NewApi")
	private void initValues() {
		SharedPreferences sp = this.activity.getSharedPreferences("FLAG",
				Context.MODE_PRIVATE);
		this.flag = sp.getBoolean("flag", false);
		this.mSwitch.setChecked(flag);

	}

	@SuppressLint("NewApi")
	private void setListeners() {
		NumButListener butListener = new NumButListener();
		SwitchChangeListener switchListener = new SwitchChangeListener();
		this.but0.setOnClickListener(butListener);
		this.but1.setOnClickListener(butListener);
		this.but2.setOnClickListener(butListener);
		this.but3.setOnClickListener(butListener);
		this.but4.setOnClickListener(butListener);
		this.but5.setOnClickListener(butListener);
		this.but6.setOnClickListener(butListener);
		this.but7.setOnClickListener(butListener);
		this.but8.setOnClickListener(butListener);
		this.but9.setOnClickListener(butListener);
		this.butDel.setOnClickListener(butListener);
		this.butClear.setOnClickListener(butListener);
		this.mSwitch.setOnCheckedChangeListener(switchListener);
	}

	// Switching, modify the flag and write it down in a configuration
	private class SwitchChangeListener implements OnCheckedChangeListener {

		@Override
		public void onCheckedChanged(CompoundButton buttonView,
				boolean isChecked) {
			flag = isChecked;
			SharedPreferences sp = activity.getSharedPreferences("FLAG",
					Context.MODE_PRIVATE);
			sp.edit().putBoolean("flag", flag).commit();
		}
	}

	private class NumButListener implements OnClickListener {

		@Override
		public void onClick(View v) {
			switch (v.getId()) {
			case R.id.but0:
				pass.append("0");
				check();
				break;
			case R.id.but1:
				pass.append("1");
				check();
				break;
			case R.id.but2:
				pass.append("2");
				check();
				break;
			case R.id.but3:
				pass.append("3");
				check();
				break;
			case R.id.but4:
				pass.append("4");
				check();
				break;
			case R.id.but5:
				pass.append("5");
				check();
				break;
			case R.id.but6:
				pass.append("6");
				check();
				break;
			case R.id.but7:
				pass.append("7");
				check();
				break;
			case R.id.but8:
				pass.append("8");
				check();
				break;
			case R.id.but9:
				pass.append("9");
				check();
				break;
			case R.id.butClear:
				pass.setText("");
				break;
			case R.id.butDel:
				pass.setText(delLastString(pass.getText().toString()));
				break;

			default:
				break;
			}
		}

		String delLastString(String s) {
			if (s.length() == 0)
				return "";
			return s.substring(0, s.length() - 1);
		}

	    //TODO: To make life easier temporarily, pwd is hardcoded :(
		void check() {
		/*	if (pass.getText().toString().equals("1234")) {
				if (flag) {
					doTrue();
				} else {
					doFalse();
				}
				TestJni jni = new TestJni();
		        jni.print("Hello JNI!"); 
				myHandler.obtainMessage(MainActivity.MSG_LOCK_SUCESS)
						.sendToTarget();
			}
		*/
			TestJni jni = new TestJni();
			jni.print(pass.getText().toString());
			if (flag) {
				doTrue();                            
			} else {
				doFalse();
			}
			myHandler.obtainMessage(MainActivity.MSG_LOCK_SUCESS).sendToTarget();
		}                                           
	}                                                   
}                                                           
                                                            
                                                            
                                                            
                                                            
