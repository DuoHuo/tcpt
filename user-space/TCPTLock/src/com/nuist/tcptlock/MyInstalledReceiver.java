package com.nuist.tcptlock;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.sax.StartElementListener;
import android.util.Log;

public class MyInstalledReceiver extends BroadcastReceiver {
	@Override
	public void onReceive(Context context, Intent intent) {

		if (intent.getAction().equals("android.intent.action.PACKAGE_ADDED")) {		// install
			String packageName = intent.getDataString();

			Log.i("homer", "安装了 :" + packageName);
		}

		if (intent.getAction().equals("android.intent.action.PACKAGE_REMOVED")) {	// uninstall
			String packageName = intent.getDataString();

			Log.i("homer", "卸载了 :" + packageName);
		}
		
		if(intent.getAction().equals("android.intent.action.BOOT_COMPLETED")) {		// boot
			Intent intent2 = new Intent(context, MainActivity.class);
			intent2.setAction("android.intent.action.MAIN");
			intent2.addCategory("android.intent.category.LAUNCHER");
			intent2.setFlags(Intent.FLAG_ACTIVITY_NEW_TASK);
			context.startActivity(intent2);
		}
	}
}