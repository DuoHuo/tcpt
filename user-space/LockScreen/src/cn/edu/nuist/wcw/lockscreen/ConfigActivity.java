package cn.edu.nuist.wcw.lockscreen;

import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.app.Activity;
import android.content.Intent;
import android.view.KeyEvent;
import android.view.Menu;
import android.view.Window;
import android.view.WindowManager;
import cn.edu.nuist.wcw.lockscreen.*;

public class ConfigActivity extends Activity {
	private MyLinearLayout layout;
	public static int MSG_LOCK_SUCESS = 1;
	private Handler mHandler = new Handler() {

		public void handleMessage(Message msg) {
			/* succeed in locking,activity distroyed */
			if (MSG_LOCK_SUCESS == msg.what)
				finish();
		}
	};

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN,
				WindowManager.LayoutParams.FLAG_FULLSCREEN);
		setContentView(R.layout.my_layout);
		this.layout = (MyLinearLayout) this.findViewById(R.id.linear);
		startService(new Intent(this, ZdLockService.class));
		layout.setParams(this, mHandler);
		layout.start();
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();
	}

	// Block 'back'
	public boolean onKeyDown(int keyCode, KeyEvent event) {
		if (event.getKeyCode() == KeyEvent.KEYCODE_BACK)
			return true;
		else
			return super.onKeyDown(keyCode, event);
	}

}
