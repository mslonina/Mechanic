#!/usr/bin/env python
# encoding: utf-8
# Thomas Nagy, 2010

"cuda"

import Task
from TaskGen import extension
import ccroot

cuda_str = '${NVCC} ${CUDAFLAGS} ${_CCINCFLAGS} ${_CCDEFFLAGS} -c ${SRC} -o ${TGT}'
cls = Task.simple_task_type('cuda', cuda_str, 'GREEN', ext_out='.o', ext_in='.c', shell=False)
cls.scan = ccroot.scan

@extension(['.cu', '.cuda'])
def c_hook(self, node):
	tsk = self.create_task('cuda', node, node.change_ext('.o'))
	self.compiled_tasks.append(tsk)
	return tsk

def detect(conf):
	conf.find_program('nvcc', var='NVCC', mandatory=True)

