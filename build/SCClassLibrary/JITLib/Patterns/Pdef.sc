StreamPlayerReference {
	
	var <>quant=1.0, <>isPlaying=false;
	var <player;
	

	*new { arg key ... argList;
		var res, stream;
		res = this.at(key);
		if(res.isNil, {
				res = super.new.initPlayer(argList);
				this.put(key, res)
		}, {
			res.setProperties(argList);
		});
		^res
	}
	
	play { arg argClock, doReset = false, argQuant;
		if (isPlaying, { "reference already playing".postln; ^this });
		player.play(argClock, doReset, argQuant ? quant);
		isPlaying = true;
	}
	
	stop {  player.stop; isPlaying = false; }
	pause { player.stop; isPlaying = false; }
	reset { player.reset }
	
	*put { this.subclassResponsibility(thisMethod) }
	*at { this.subclassResponsibility(thisMethod) }
	setProperties { this.subclassResponsibility(thisMethod) }
	initPlayer { this.subclassResponsibility(thisMethod) }
	sched { arg func; player.clock.sched({ func.value; nil }, quant) }

}


Tdef : StreamPlayerReference {
	classvar <>all;
	
	*initClass {
		CmdPeriod.add(this);
		this.clear;
	}
	*cmdPeriod { this.all.do({ arg item; item.stop }) }
	
	*remove { arg key;
		this.all.removeAt(key).stop;
	}
	*removeAll { this.all.do({ arg item; item.stop }); this.clear }
	*clear { all = () }
	*at { arg key;
		^this.all.at(key)
	}
	*put { arg key, obj;
		this.all.put(key, obj)
	}
	initPlayer { arg argList;
		var func;
		#func = argList;
		player = Task.new(func ? { 1.yield });
	}
	
	setProperties { arg argList;
		var func;
		#func = argList;
		if(func.notNil, { 
			this.stream = Routine.new(func);
		})
	}
	 
	stream_ { arg str;
			if(isPlaying)
				{ 
					if(player.isPlaying)
					 { player.clock.play({ player.stream = str; nil })  }
			 		 { player.stream = str; player.play(quant: quant) }
				}
			 	{ 
			 		player.stream = str
			 	}
	}


}

Pdef : Tdef {
	classvar <all, <>defaultEvent, <defaultStream;
	
	*initClass {
		CmdPeriod.add(this);
		this.clear;
		defaultStream = Pbind(\freq, \rest).asStream;
	}
	*clear { all = () }
	
	initPlayer { arg argList;
		var pat, event, stream;
		#pat, event = argList;
		player = EventStreamPlayer.new(
			stream.asStream ? this.class.defaultStream, 
			event ? this.class.defaultEvent
		);
	}
	
	setProperties { arg argList;
		var pat, event, stream;
		#pat, event = argList;
		stream = pat.asStream;
		if(event.notNil, { player.event = event });
		if(stream.notNil, { this.stream = stream });
	}
	
	event_ { arg inEvent; player.event = inEvent }
	mute { player.mute }
	unmute { player.unmute }
	
}

