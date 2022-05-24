"""
    Copyright (c) 2022, Alibaba Group Holding Limited;
    
    Licensed under the Apache License, Version 2.0 (the "License");
    you may not use this file except in compliance with the License.
    You may obtain a copy of the License at
    
        http://www.apache.org/licenses/LICENSE-2.0
    
    Unless required by applicable law or agreed to in writing, software
    distributed under the License is distributed on an "AS IS" BASIS,
    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
    See the License for the specific language governing permissions and
    limitations under the License.
"""

import gdb
from gdb.FrameDecorator import FrameDecorator
import traceback

"""
IMPORTANT: This scripts assumes that all the promise_type have the 
           continuation field as the first data member.
"""

class SymValueWrapper():
    def __init__(self, symbol, value):
        self.sym = symbol
        self.val = value

    def value(self):
        return self.val

    def symbol(self):
        return self.sym

    def __str__(self):
        return str(self.sym) + " = " + str(self.val)

class LazyGetType(object):
    def __init__(self, cls):
        self.cls = cls
    def __getattr__(self, name):
        if name.endswith('_size'):
            value = getattr(self, name[:-len('_size')]).sizeof
        elif name.endswith('_pointer'):
            value = getattr(self, name[:-len('_pointer')]).pointer()
        else:
            value = gdb.lookup_type(self.cls.type_map[name])
        setattr(self, name, value)
        return value

@LazyGetType
class types(object):
    type_map = {'long': 'long',
                'promise_base': 'async_simple::coro::detail::LazyPromiseBase',
                'continuation': 'std::coroutine_handle<void>'
               }

class CoroutineFrame(object):
    def __init__(self, task_addr):
        self.promise_base_addr = task_addr + types.long_pointer_size * 2
        # In async_simple, the continuation address should be equal to promise_address
        self.continuation_addr = self.promise_base_addr

    def next_task_addr(self):
        return dereference(self.continuation_addr)

    def __getattr__(self, name):
        if name.endswith("_pointer"):
            name = name[:-len("_pointer")]
            return cast_addr2long_pointer(object.__getattribute__(self, name))
        print "Search: ", name
        return object.__getattribute__(self, name)

def cast_addr2long_pointer(addr):
    return gdb.Value(addr).cast(types.long_pointer)

def dereference(addr):
    return long(cast_addr2long_pointer(addr).dereference())

class StacklessCoroutineFrame(CoroutineFrame):
    def __init__(self, task_addr):
        super(StacklessCoroutineFrame, self).__init__(task_addr)

        self.resume_addr_size = types.long_pointer_size
        self.destroy_addr_size = types.long_pointer_size

        self.promise_type_addr = self.promise_base_addr
        self.destroy_addr = self.promise_type_addr - self.destroy_addr_size
        self.resume_addr = self.destroy_addr - self.resume_addr_size
        self.frame_addr = self.resume_addr
        self.other_fields = self.continuation_addr + types.continuation_size

class StacklessCoroutineFrameDecorator(FrameDecorator):
    def __init__(self, frame, coro_frame):
        super(StacklessCoroutineFrameDecorator, self).__init__(frame)
        self.coro_frame = coro_frame

        self.resume_func = dereference(self.coro_frame.resume_addr)
        self.resume_func_block = gdb.block_for_pc(self.resume_func)
        if self.resume_func_block == None:
            raise Exception('Not stackless coroutine.')
        self.line_info = gdb.find_pc_line(self.resume_func)

    def address(self):
        return self.resume_func

    def filename(self):
        return self.line_info.symtab.filename

    def frame_args(self):
        return [SymValueWrapper("frame_addr", self.coro_frame.frame_addr_pointer),
                SymValueWrapper("promise_addr", self.coro_frame.promise_base_addr_pointer),
                SymValueWrapper("continuation_addr", self.coro_frame.continuation_addr_pointer),
                SymValueWrapper("others_addr", self.coro_frame.other_fields_pointer)
                ]

    def function(self):
        return self.resume_func_block.function.print_name

    def func_linkage_name(self):
        return self.resume_func_block.function.linkage_name

    def line(self):
        return self.line_info.line

class StripDecorator(FrameDecorator):
    def __init__(self, frame):
        super(StripDecorator, self).__init__(frame)
        self.frame = frame
        f = frame.function()
        self.function_name = f
        self.elided_frames = None
        self.linkage_name = frame.func_linkage_name() + ".coro_frame_ty"
        if not isinstance(f, str):
            self.template_arguments = []
            return

    def __str__(self, shift = 2):
        addr = "" if self.address() == None else '%#x' % self.address() + " in "
        location = "" if self.filename() == None else " at " + self.filename() + ":" + str(self.line())
        return addr + self.function() + " " + str([str(args) for args in self.frame_args()]) + location

    def frame_type_str(self):
        return self.linkage_name

class CoroutineFilter:
    def create_coroutine_frames(self, frame, task_addr, append_first = False):
        if append_first:
            next_task_addr = task_addr
        else:
            coro_frame = CoroutineFrame(task_addr)
            next_task_addr = coro_frame.next_task_addr()
        frames = []
        while next_task_addr != 0:
            try:
                coro_frame = StacklessCoroutineFrame(next_task_addr)
                frames.append(StacklessCoroutineFrameDecorator(frame, coro_frame))
            except Exception as e:
                traceback.print_exc()
                return frames
            next_task_addr = coro_frame.next_task_addr()
        return frames

class LazyStack(gdb.Command):
    def __init__(self):
        super(LazyStack, self).__init__("lazy-bt", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        coroutine_filter = CoroutineFilter()
        argv = gdb.string_to_argv(arg)
        if len(argv) == 0:
            try:
                task = gdb.parse_and_eval('__coro_frame')
                task = int(str(task.address), 16)
            except Exception:
                print ("Can't find __coro_frame in current context.\n" +
                       "Please use `lazy-bt` in stackless coroutine context.")
                return
        elif len(argv) != 1:
            print("usage: lazy-bt <pointer to task>")
            return
        else:
            task = int(argv[0], 16)
        frames = coroutine_filter.create_coroutine_frames(None, task, True)
        i = 0
        for f in frames:
            print '#'+ str(i), str(StripDecorator(f))
            i += 1
        return

LazyStack()

class ShowCoroFrame(gdb.Command):
    def __init__(self):
        super(ShowCoroFrame, self).__init__("show-coro-frame", gdb.COMMAND_USER)

    def invoke(self, arg, from_tty):
        argv = gdb.string_to_argv(arg)
        if len(argv) != 1:
            print("usage: show-coro-frame <address of coroutine frame>")
            return

        addr = int(argv[0], 16)
        block = gdb.block_for_pc(long(gdb.Value(addr).cast(types.long_pointer).dereference()))
        if block == None:
            print "block " + str(addr) + "  is none."
            return

        # Disable demangling since gdb will treat names starting with `_Z`(The marker for Itanium ABI) specially.
        gdb.execute("set demangle-style none")

        Decorator = StacklessCoroutineFrameDecorator(None, StacklessCoroutineFrame(addr))
        StrippedDecorator = StripDecorator(Decorator)
        coro_frame_ptr_type = gdb.lookup_type(StrippedDecorator.frame_type_str()).pointer()
        coro_frame = gdb.Value(addr).cast(coro_frame_ptr_type).dereference()

        gdb.execute("set demangle-style auto")
        gdb.write(coro_frame.format_string(pretty_structs = True))

ShowCoroFrame()
