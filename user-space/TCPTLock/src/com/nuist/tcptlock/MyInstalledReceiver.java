package com.nuist.tcptlock;

import android.app.KeyguardManager;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.sax.StartElementListener;

public class MyInstalledReceiver extends BroadcastReceiver {

	private KeyguardManager keyguardManager;
	private KeyguardManager.KeyguardLock keyguardLock;

	@Override
	public void onReceive(Context context, Intent intent) {
		// TODO Auto-generated method stub
		String action = intent.getAction();

		System.out.println("if 外面");
		if (action.equals(Intent.ACTION_BOOT_COMPLETED)) {
			System.out.println("if 里面");
            //TODO



			Intent lockIntent = new Intent(context, MainActivity.class);
			lockIntent.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			keyguardManager = (KeyguardManager) context
					.getSystemService(Context.KEYGUARD_SERVICE);
			keyguardLock = keyguardManager.newKeyguardLock("");
			keyguardLock.disableKeyguard();

			context.startActivity(lockIntent);


		}

	}

}
