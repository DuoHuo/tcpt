package com.nuist.tcptlock;


import android.app.KeyguardManager;
import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.os.IBinder;
import android.view.Window;

public class LockScreenService extends Service {

	private Intent startIntent = null;

	@Override
	public IBinder onBind(Intent arg0) {
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public void onCreate() {
		// TODO Auto-generated method stub

		super.onCreate();
		startIntent = new Intent(LockScreenService.this, MainActivity.class);
		startIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);

		IntentFilter filter = new IntentFilter();
		filter.addAction(Intent.ACTION_SCREEN_OFF);
		LockScreenService.this.registerReceiver(MyLockScreenReceiver, filter);
		System.out.println("onCreate()................");
	}

	@Override
	public void onStart(Intent intent, int startId) {
		// TODO Auto-generated method stub
		super.onStart(intent, startId);
	}
	
	@Override
	public int onStartCommand(Intent intent, int flags, int startId) {
		return Service.START_STICKY;
	}

	@Override
	public void onDestroy() {
		// TODO Auto-generated method stub
		super.onDestroy();
		System.out.println("onDestroy()...........");
		//注销广播
		unregisterReceiver(MyLockScreenReceiver);
		//再次启动服务
		startService(new Intent(LockScreenService.this, LockScreenService.class));
	}
	//取消系统默认锁屏界面的关键对象
	private KeyguardManager keyguardManager;
	private KeyguardManager.KeyguardLock keyguardLock;

	private BroadcastReceiver MyLockScreenReceiver = new BroadcastReceiver() {

		@Override
		public void onReceive(Context context, Intent intent) {
			// TODO Auto-generated method stub
			System.out.println("receiver--action="+intent.getAction());
			/**
			 * ACTION_SCREEN_OFF表示按下电源键，屏幕黑屏
			 * ACTION_SCREEN_ON 屏幕黑屏情况下，按下电源键
			 */
			if (intent.getAction().equals(Intent.ACTION_SCREEN_OFF)
					|| intent.getAction().equals(Intent.ACTION_SCREEN_ON)) {
				System.out.println("receiver.................");
				keyguardManager = (KeyguardManager) context
						.getSystemService(Context.KEYGUARD_SERVICE);
				// 取消默认的锁屏
				keyguardLock = keyguardManager.newKeyguardLock("");
				keyguardLock.disableKeyguard();

				startActivity(startIntent);

			}
		}
	};

}