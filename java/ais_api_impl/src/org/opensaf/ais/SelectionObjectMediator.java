/*      -*- OpenSAF  -*-
 *
 * (C) Copyright 2008 The OpenSAF Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. This file and program are licensed
 * under the GNU Lesser General Public License Version 2.1, February 1999.
 * The complete license can be accessed from the following location:
 * http://opensource.org/licenses/lgpl-license.php
 * See the Copying file included with the OpenSAF distribution for full
 * licensing terms.
 *
 * Author(s): Nokia Siemens Networks, OptXware Research & Development LLC.
 */

package org.opensaf.ais;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.Pipe;
import java.nio.channels.SelectableChannel;
import java.nio.channels.Pipe.SourceChannel;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;

/**
 * A singleton that dispatches calls between the callbacks of the actual SAForum
 * implementation and Handle instances.
 */
public class SelectionObjectMediator {

	private static final SelectionObjectMediator INSTANCE = new SelectionObjectMediator();

	/**
	 * Mutex used for synchronizing concurrent accesses to the metadata of selection objects.
	 */
	private final Object mutex = new Object();

	/**
	 * Map of selection objects and their metadata and the Pipe instances
	 * associated to them.
	 */
	private Map<Long, HandleData> selectionObjects;

	/**
	 * File descriptors (sink, source) of a socket pair used for multiplexing
	 * the processing of callbacks.
	 */
	private int[] notifierSocketpair;

	private Worker worker;

	/**
	 * Static factory method returns the already-created instance of this class.
	 * @return An instance of this class.
	 */
	public static SelectionObjectMediator getInstance() {
		return INSTANCE;
	}

	/**
	 * Creates an instance of this class.
	 */
	private SelectionObjectMediator() {
		selectionObjects = new ConcurrentHashMap<Long, HandleData>();

		notifierSocketpair = openSocketpair();

		worker = new Worker();
		Thread t = new Thread(worker);
		t.setDaemon(true);
		t.start();
	}

	/**
	 * Registers the given selection object for processing. The worker thread
	 * will call select() on it and manages the Pipe instances (reads and writes
	 * the source and sink channels) associated to it.
	 *
	 * @param selectionObject
	 *            The selection object in which the worker will listen on.
	 */
	public void addHandle(long selectionObject) throws IOException {
		if (selectionObjects.get(selectionObject) == null) {
			Pipe pipe = Pipe.open();
			HandleData h = new HandleData();
			h.setSelectionObject(selectionObject);
			h.setPipe(pipe);
			h.setActive(true);
			synchronized (mutex) {
				selectionObjects.put(selectionObject, h);
			}
			// causes select() to return immediately
			writeSocketpair(notifierSocketpair[0], new Long(-1).intValue());
		}
	}

	/**
	 * Returns the channel associated to the given selection object.
	 *
	 * @param selectionObject
	 *            The selection object the channel is assigned to.
	 * @return An instance of {@link SelectableChannel} associated to the
	 *         selection object.
	 */
	public SelectableChannel getAssociatedChannel(long selectionObject) {
		SourceChannel channel = null;

		HandleData h = selectionObjects.get(selectionObject);
		if (h != null) {
			channel = h.getPipe().source();
		}

		return channel;
	}

	/**
	 * Cancels the further processing associated to the given selection object.
	 * The worker thread will not call <i>select</i> on it in the future.
	 *
	 * @param selectionObject
	 *            The selection object associated to the handle to be removed.
	 */
	public void removeHandle(long selectionObject) throws IOException {
		HandleData h = selectionObjects.get(selectionObject);

		synchronized (mutex) {
			selectionObjects.remove(selectionObject);
		}
		if (h != null) {
			h.getPipe().source().close();
			h.getPipe().sink().close();
		}
		// causes select() to return immediately
		writeSocketpair(notifierSocketpair[0], new Long(selectionObject).intValue());
	}

	/**
	 * Called by {@link Handle} instances to indicate that dispatch is finished.
	 * @param selectionObject
	 *            The selection object associated to the handle.
	 */
	public void pendingCallbacksDispatched(long selectionObject)
			throws IOException {
		synchronized (mutex) {
			HandleData h = selectionObjects.get(selectionObject);
			if (h != null) {
				clearPipe(h.getPipe());
				writeSocketpair(notifierSocketpair[0],
						new Long(h.getSelectionObject()).intValue());
			}
		}
	}

	/**
	 * Represents the metadata and the {@link Pipe} instance associated to
	 * selection objects.
	 */
	private class HandleData {

		private long selectionObject;

		private boolean active;

		private Pipe pipe;

		public long getSelectionObject() {
			return selectionObject;
		}

		public void setSelectionObject(long selectionObject) {
			this.selectionObject = selectionObject;
		}

		public boolean isActive() {
			return active;
		}

		public void setActive(boolean active) {
			this.active = active;
		}

		public Pipe getPipe() {
			return pipe;
		}

		public void setPipe(Pipe pipe) {
			this.pipe = pipe;
		}

	}

	/**
	 * Worker that processes the metadata and calls select() on active selection
	 * objects and manages the Pipe instances (reads and writes the source and
	 * sink channels) associated to them.
	 */
	private class Worker implements Runnable {

		public void run() {
			while (true) {
				ArrayList<Integer> objects = new ArrayList<Integer>();

				synchronized (mutex) {
					Set<Long> keys = selectionObjects.keySet();
					Iterator<Long> iter = keys.iterator();
					while (iter.hasNext()) {
						Long key = iter.next();
						if (selectionObjects.get(key).isActive()) {
							objects.add(key.intValue());
						}
					}
				}
				int[] so = new int[objects.size() + 1];
				so[0] = notifierSocketpair[1];
				for (int i = 1; i < so.length; i++) {
					so[i] = objects.get(i - 1);
				}

//				System.out.println("JAVA WORKER: Sockets for select: " + intArrayToString(so));
//				System.out.flush();
				int[] returnedSelectionObjects = doSelect(so, so.length);


				if (returnedSelectionObjects != null) {
//					System.out.println("JAVA WORKER: Sockets returned from select: " + intArrayToString(returnedSelectionObjects));
//					System.out.flush();
					synchronized (mutex) {
						for (Integer rso : returnedSelectionObjects) {
							HandleData h = selectionObjects.get(rso.longValue());
							if (h != null) {
//								System.out.println("JAVA WORKER: Event on socket: " + h.getSelectionObject());
//								System.out.flush();
								// write current selectionObject to pipe
								ByteBuffer src = ByteBuffer.allocate(Integer.SIZE / 8);
								src.putInt(rso);
								src.flip();
								try {
//									System.out.println("JAVA WORKER: Writing to pipe (" + h.getSelectionObject() +")");
//									System.out.flush();
									int ret = h.getPipe().sink().write(src);
									if (ret == 0) {
//										System.out.println("JAVA WORKER: Writing to pipe failed.");
//										System.out.flush();
									}
//									System.out.println("JAVA WORKER: Deactivating selection object (" + h.getSelectionObject() +")");
//									System.out.flush();
									h.setActive(false);
								} catch (IOException e) {
									// TODO clarify logging mechanism
									e.printStackTrace();
								}
							} else if (rso == notifierSocketpair[1]) {
//								System.out.println("JAVA WORKER: Event on notifier socketpair (" + rso +")");
//								System.out.flush();
								int fd = readSocketpair(notifierSocketpair[1]);
//								System.out.println("JAVA WORKER: Event on socketpair: socket value: " + fd);
//								System.out.flush();
								h = selectionObjects.get(new Integer(fd).longValue());
								if (h != null) {
//									System.out.println("JAVA WORKER: Event on socketpair: activating socket: " + fd);
//									System.out.flush();
									h.setActive(true);
								} else {
//									System.out.println("JAVA WORKER: Event on socketpair: only wake up signal");
//									System.out.flush();
								} // if
							} // if
						} // for
					} // synchronized
				} // if
			} // while
		} // run()

		/**
		 * Calls <i>select</i> on the given selection objects and returns an
		 * array of the modified ones. In case of any error, it returns NULL.
		 */
		private native int[] doSelect(int[] selectionObjects, int numberOfselectionObjects);

		/**
		 * Creates a comma-separated string from the given array of integers.
		 */
		private String intArrayToString(int[] array) {
			String ret = "";
			for (int i = 0; i < array.length; i++) {
				ret += array[i] + ",";
			}
			return ret;
		}

	}

	/**
	 * Reads out the source channel or the given {@link Pipe} instance.
	 */
	private void clearPipe(Pipe pipe) throws IOException {
		SourceChannel s = pipe.source();
		ByteBuffer dst = ByteBuffer.allocate(128);
		int num = -2;
		while (num < 1) {
		//	System.out.println("SOM: Clearing pipe.");
			num = s.read(dst);
			dst.clear();
		}
	}

	/**
	 * Opens a socket pair and returns the file descriptors of the sink and the source.
	 */
	private native int[] openSocketpair();

	/**
	 * Closes the file descriptors of a socket pair.
	 *
	 * Not used
	 */
	private native void closeSocketpair(int[] sockets);

	/**
	 * Reads an integer from a given socket which is the source of a socket pair.
	 */
	private native int readSocketpair(int source);

	/**
	 * Writes an integer the the given socket which is the sink of a socket pair.
	 */
	private native void writeSocketpair(int sink, int data);

}
