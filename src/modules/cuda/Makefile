# Copyright (C) 2011 Jason Mercer.  All rights reserved.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
#

DIRS=array_cuda anisotropy_cuda appliedfield_cuda core_cuda \
     longrange2d_cuda longrange3d_cuda	magnetostatics2d_cuda \
     magnetostatics3d_cuda random_cuda \
     exchange_cuda llg_cuda thermal_cuda

all:
	@for d in $(DIRS); do (make -C $$d); done

install:
	@for d in $(DIRS); do (make -C $$d install); done

uninstall:
	@for d in $(DIRS); do (make -C $$d uninstall); done

clean:
	@for d in $(DIRS); do (make -C $$d clean); done
	rm -f *~

distclean:
	@for d in $(DIRS); do (make -C $$d distclean); done
	rm -f *~

