from Build import BuildContext
import pproc

def _log_exec_command(s, **kw):
	"""Like pproc.exec_command, but returns both return code as well as
	stdout/stderr."""
	if 'log' in kw:
		kw["stdout"] = pproc.PIPE
		kw["stderr"] = pproc.STDOUT
		log = kw["log"]
		del kw["log"]
	kw['shell'] = isinstance(s, str)

	p = pproc.Popen(s, **kw)
	ret = p.wait()
	lout = p.communicate()[0]
	log.write(lout)
	
	return ret, lout

class MyBuildContext(BuildContext):
	"""A build context whose log output can be queried."""
	def __init__(self):
		self.out = None
		BuildContext.__init__(self) 

	def exec_command(self, cmd, **kw):
		# 'runner' zone is printed out for waf -v, see wafadmin/Options.py
		if self.log:
			self.log.write('%s\n' % cmd)
			kw['log'] = self.log
		try:
			if not 'cwd' in kw: kw['cwd'] = self.cwd
		except AttributeError:
			self.cwd = kw['cwd'] = self.bldnode.abspath()
		ret, self.out = _log_exec_command(cmd, **kw)
		return ret
