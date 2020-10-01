var r=new String("?");
if (GSSchedulerEngine)
{
  var myMI=GSSystemUtils.GetMemoryInfo(false, false);

  // enhance me: find/guess the max mem for a process for the current platform/build
	//if (GSSystemUtils.GetPlaform().substr(GSSystemUtils.GetPlaform().length-2,GSSystemUtils.GetPlaform().length)=='32')
	// limit=2000 (Mo) else limit = ?
	
  if (myMI["MemUsedByProcess"] > 0.9 * myMI["TotalPhysMem"])
  {	
	if (GSSchedulerEngine.GetStatusTask())
  	{
		r=GSSchedulerEngine.SendEmail( GSSchedulerEngine.GetAutoAdminTask().GetAttribute('EmailFrom'), GSSchedulerEngine.GetAutoAdminTask().GetAttribute('Emails'), 
			'Low memory', GSEngine.Get("AppFullName")+' has reached a low memeory state: MemUsedByProcess='+myMI["MemUsedByProcess"] 
		+ '  TotalPhysMem=' + myMI["TotalPhysMem"], false, '' );
	}
	else
		r="error: no status task";
  }
  else r="ok: current used mem: "+myMI["MemUsedByProcess"];
}
else
	r="error: no GSSchedulerEngine";
r;
