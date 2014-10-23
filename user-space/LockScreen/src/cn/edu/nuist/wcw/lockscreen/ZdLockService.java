package cn.edu.nuist.wcw.lockscreen;

import android.app.KeyguardManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.util.Log;
import android.widget.Toast;


public class ZdLockService extends Service {

	private static String TAG = "ZdLockService";
	private Intent zdLockIntent = null;

	/* When unlocking the screen */
	
	public void openScreen() {  
	    /** TODO:
	     * It's another good idea to set crypt flag here
	     * so that we can use system default lock screen
	     */ 
		Log.i(TAG, "unlocking");
	}

	/* When locking the screen */
	public void closeScreen() { //TODO
		Log.i(TAG, "locking");
		TestJni jni = new TestJni();
		jni.brint("Hello JNI!"); 
	}

	@Override
	public IBinder onBind(Intent arg0) {
		return null;
	}

	public void onCreate() {
		super.onCreate();

		zdLockIntent = new Intent(ZdLockService.this, MainActivity.class);
		zdLockIntent.addFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

		/* Registering broadcast */
		IntentFilter mScreenOnFilter = new IntentFilter(
				"android.intent.action.SCREEN_ON");
		ZdLockService.this.registerReceiver(mScreenOnReceiver, mScreenOnFilter);

		/* Registering broadcast */
		IntentFilter mScreenOffFilter = new IntentFilter(
				"android.intent.action.SCREEN_OFF");
		ZdLockService.this.registerReceiver(mScreenOffReceiver,
				mScreenOffFilter);
	}

	public int onStartCommand(Intent intent, int flags, int startId) {
		return Service.START_STICKY;

	}

	public void onDestroy() {
		super.onDestroy();
		ZdLockService.this.unregisterReceiver(mScreenOnReceiver);
		ZdLockService.this.unregisterReceiver(mScreenOffReceiver);
		/* Restart */
		startService(new Intent(ZdLockService.this, ZdLockService.class));
	}

	private KeyguardManager mKeyguardManager = null;
	private KeyguardManager.KeyguardLock mKeyguardLock = null;
	// Hide the system default locking screen
	private BroadcastReceiver mScreenOnReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {

			Log.i(TAG, intent.getAction() + "by wcw.");

			if (intent.getAction().equals("android.intent.action.SCREEN_ON")) {
				// Log.i(TAG, "--- android.intent.action.SCREEN_ON------");
				openScreen();
			}
		}
	};

	// Unlock the system locking screen so we can use ours
	private BroadcastReceiver mScreenOffReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			String action = intent.getAction();
			// Log.i(TAG, intent.toString() + "by wcw3.");
			closeScreen();

			if (action.equals("android.intent.action.SCREEN_OFF")
					|| action.equals("android.intent.action.SCREEN_ON")) {
				mKeyguardManager = (KeyguardManager) context
						.getSystemService(Context.KEYGUARD_SERVICE);
				mKeyguardLock = mKeyguardManager.newKeyguardLock("zdLock 1");
				mKeyguardLock.disableKeyguard();
				startActivity(zdLockIntent);
			}
		}
	};

}
