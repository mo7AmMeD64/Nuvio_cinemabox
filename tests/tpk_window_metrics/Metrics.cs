using System;
using NuvioTpk;
class Test
{
    static void Assert(bool value, string message) { if (!value) throw new Exception(message); }
    static void Call(VideoWindowMetrics m, object player, int x, int y, int w, int h, bool full, bool success=true) {
        m.Begin(player,x,y,w,h,full);m.Complete(success,10);
    }
    static void Main() {
        var m=new VideoWindowMetrics();var player=new object();
        for(int i=0;i<10000;i++)Call(m,player,0,0,1920,1080,true);
        Assert(m.Requests==10000 && m.Applied==10000 && m.RepeatedRequests==9999,"explicit repeated requests must all remain applied");
        Console.WriteLine($"fullscreen: requests={m.Requests} repeated={m.RepeatedRequests} applied={m.Applied} native_call_reduction=0");
        m.Reset();
        for(int i=0;i<4;i++)Call(m,player,40,40,1200,675,false);
        Assert(m.Applied==4 && m.RepeatedRequests==3,"preserve the trailer's deliberate ROI recovery calls");
        Call(m,player,40,40,1200,675,false,false);
        long before=m.RepeatedRequests;
        Call(m,player,40,40,1200,675,false);
        Assert(m.Failed==1 && m.Applied==5 && m.RepeatedRequests==before,"failed native work does not suppress or count retry as redundant");
        Call(m,player,0,0,1920,1080,true);before=m.RepeatedRequests;
        Call(m,new object(),0,0,1920,1080,true);
        Assert(m.RepeatedRequests==before,"new player invalidates repeated geometry baseline");
        m.Invalidate();before=m.RepeatedRequests;Call(m,player,0,0,1920,1080,true);
        Assert(m.RepeatedRequests==before,"pipeline transition invalidation");
        Assert(m.ElapsedTicks==m.Requests*10 && m.MaxTicks==10,"duration counters");
        m.Reset();Assert(m.Requests==0 && m.Applied==0 && m.RepeatedRequests==0,"session counters reset");
        Console.WriteLine("tpk window metrics: ROI retries, failures, identity, transitions and session resets passed");
    }
}
