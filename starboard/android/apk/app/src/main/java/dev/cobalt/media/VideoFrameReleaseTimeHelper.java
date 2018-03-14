/*
 * Copyright (C) 2016 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
// Modifications Copyright 2017 Google Inc. All Rights Reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

package dev.cobalt.media;

import android.annotation.TargetApi;
import android.content.Context;
import android.os.Handler;
import android.os.HandlerThread;
import android.os.Message;
import android.view.Choreographer;
import android.view.Choreographer.FrameCallback;
import android.view.WindowManager;
import dev.cobalt.util.UsedByNative;

/** Makes a best effort to adjust frame release timestamps for a smoother visual result. */
@TargetApi(16)
@SuppressWarnings("unused")
@UsedByNative
public final class VideoFrameReleaseTimeHelper {

  private static final double DISPLAY_REFRESH_RATE_UNKNOWN = -1;
  private static final long CHOREOGRAPHER_SAMPLE_DELAY_MILLIS = 500;
  private static final long MAX_ALLOWED_DRIFT_NS = 20000000;

  private static final long VSYNC_OFFSET_PERCENTAGE = 80;
  private static final int MIN_FRAMES_FOR_ADJUSTMENT = 6;
  private static final long NANOS_PER_SECOND = 1000000000L;

  private final VSyncSampler vsyncSampler;
  private final boolean useDefaultDisplayVsync;
  private final long vsyncDurationNs;
  private final long vsyncOffsetNs;

  private long lastFramePresentationTimeUs;
  private long adjustedLastFrameTimeNs;
  private long pendingAdjustedFrameTimeNs;

  private boolean haveSync;
  private long syncUnadjustedReleaseTimeNs;
  private long syncFramePresentationTimeNs;
  private long frameCount;

  /**
   * Constructs an instance that smooths frame release timestamps but does not align them with the
   * default display's vsync signal.
   */
  @SuppressWarnings("unused")
  public VideoFrameReleaseTimeHelper() {
    this(DISPLAY_REFRESH_RATE_UNKNOWN);
  }

  /**
   * Constructs an instance that smooths frame release timestamps and aligns them with the default
   * display's vsync signal.
   *
   * @param context A context from which information about the default display can be retrieved.
   */
  @SuppressWarnings("unused")
  @UsedByNative
  public VideoFrameReleaseTimeHelper(Context context) {
    this(getDefaultDisplayRefreshRate(context));
  }

  private VideoFrameReleaseTimeHelper(double defaultDisplayRefreshRate) {
    useDefaultDisplayVsync = defaultDisplayRefreshRate != DISPLAY_REFRESH_RATE_UNKNOWN;
    if (useDefaultDisplayVsync) {
      vsyncSampler = VSyncSampler.getInstance();
      vsyncDurationNs = (long) (NANOS_PER_SECOND / defaultDisplayRefreshRate);
      vsyncOffsetNs = (vsyncDurationNs * VSYNC_OFFSET_PERCENTAGE) / 100;
    } else {
      vsyncSampler = null;
      vsyncDurationNs = -1; // Value unused.
      vsyncOffsetNs = -1; // Value unused.
    }
  }

  /** Enables the helper. */
  @SuppressWarnings("unused")
  @UsedByNative
  public void enable() {
    haveSync = false;
    if (useDefaultDisplayVsync) {
      vsyncSampler.addObserver();
    }
  }

  /** Disables the helper. */
  @SuppressWarnings("unused")
  @UsedByNative
  public void disable() {
    if (useDefaultDisplayVsync) {
      vsyncSampler.removeObserver();
    }
  }

  /**
   * Adjusts a frame release timestamp.
   *
   * @param framePresentationTimeUs The frame's presentation time, in microseconds.
   * @param unadjustedReleaseTimeNs The frame's unadjusted release time, in nanoseconds and in the
   *     same time base as {@link System#nanoTime()}.
   * @return The adjusted frame release timestamp, in nanoseconds and in the same time base as
   *     {@link System#nanoTime()}.
   */
  @SuppressWarnings("unused")
  @UsedByNative
  public long adjustReleaseTime(long framePresentationTimeUs, long unadjustedReleaseTimeNs) {
    long framePresentationTimeNs = framePresentationTimeUs * 1000;

    // Until we know better, the adjustment will be a no-op.
    long adjustedFrameTimeNs = framePresentationTimeNs;
    long adjustedReleaseTimeNs = unadjustedReleaseTimeNs;

    if (haveSync) {
      // See if we've advanced to the next frame.
      if (framePresentationTimeUs != lastFramePresentationTimeUs) {
        frameCount++;
        adjustedLastFrameTimeNs = pendingAdjustedFrameTimeNs;
      }
      if (frameCount >= MIN_FRAMES_FOR_ADJUSTMENT) {
        // We're synced and have waited the required number of frames to apply an adjustment.
        // Calculate the average frame time across all the frames we've seen since the last sync.
        // This will typically give us a frame rate at a finer granularity than the frame times
        // themselves (which often only have millisecond granularity).
        long averageFrameDurationNs =
            (framePresentationTimeNs - syncFramePresentationTimeNs) / frameCount;
        // Project the adjusted frame time forward using the average.
        long candidateAdjustedFrameTimeNs = adjustedLastFrameTimeNs + averageFrameDurationNs;

        if (isDriftTooLarge(candidateAdjustedFrameTimeNs, unadjustedReleaseTimeNs)) {
          haveSync = false;
        } else {
          adjustedFrameTimeNs = candidateAdjustedFrameTimeNs;
          adjustedReleaseTimeNs =
              syncUnadjustedReleaseTimeNs + adjustedFrameTimeNs - syncFramePresentationTimeNs;
        }
      } else {
        // We're synced but haven't waited the required number of frames to apply an adjustment.
        // Check drift anyway.
        if (isDriftTooLarge(framePresentationTimeNs, unadjustedReleaseTimeNs)) {
          haveSync = false;
        }
      }
    }

    // If we need to sync, do so now.
    if (!haveSync) {
      syncFramePresentationTimeNs = framePresentationTimeNs;
      syncUnadjustedReleaseTimeNs = unadjustedReleaseTimeNs;
      frameCount = 0;
      haveSync = true;
      onSynced();
    }

    lastFramePresentationTimeUs = framePresentationTimeUs;
    pendingAdjustedFrameTimeNs = adjustedFrameTimeNs;

    if (vsyncSampler == null || vsyncSampler.sampledVsyncTimeNs == 0) {
      return adjustedReleaseTimeNs;
    }

    // Find the timestamp of the closest vsync. This is the vsync that we're targeting.
    long snappedTimeNs =
        closestVsync(adjustedReleaseTimeNs, vsyncSampler.sampledVsyncTimeNs, vsyncDurationNs);
    // Apply an offset so that we release before the target vsync, but after the previous one.
    return snappedTimeNs - vsyncOffsetNs;
  }

  protected void onSynced() {
    // Do nothing.
  }

  private boolean isDriftTooLarge(long frameTimeNs, long releaseTimeNs) {
    long elapsedFrameTimeNs = frameTimeNs - syncFramePresentationTimeNs;
    long elapsedReleaseTimeNs = releaseTimeNs - syncUnadjustedReleaseTimeNs;
    return Math.abs(elapsedReleaseTimeNs - elapsedFrameTimeNs) > MAX_ALLOWED_DRIFT_NS;
  }

  private static long closestVsync(long releaseTime, long sampledVsyncTime, long vsyncDuration) {
    long vsyncCount = (releaseTime - sampledVsyncTime) / vsyncDuration;
    long snappedTimeNs = sampledVsyncTime + (vsyncDuration * vsyncCount);
    long snappedBeforeNs;
    long snappedAfterNs;
    if (releaseTime <= snappedTimeNs) {
      snappedBeforeNs = snappedTimeNs - vsyncDuration;
      snappedAfterNs = snappedTimeNs;
    } else {
      snappedBeforeNs = snappedTimeNs;
      snappedAfterNs = snappedTimeNs + vsyncDuration;
    }
    long snappedAfterDiff = snappedAfterNs - releaseTime;
    long snappedBeforeDiff = releaseTime - snappedBeforeNs;
    return snappedAfterDiff < snappedBeforeDiff ? snappedAfterNs : snappedBeforeNs;
  }

  private static double getDefaultDisplayRefreshRate(Context context) {
    WindowManager manager = (WindowManager) context.getSystemService(Context.WINDOW_SERVICE);
    return manager.getDefaultDisplay() != null
        ? manager.getDefaultDisplay().getRefreshRate()
        : DISPLAY_REFRESH_RATE_UNKNOWN;
  }

  /**
   * Samples display vsync timestamps. A single instance using a single {@link Choreographer} is
   * shared by all {@link VideoFrameReleaseTimeHelper} instances. This is done to avoid a resource
   * leak in the platform on API levels prior to 23.
   */
  private static final class VSyncSampler implements FrameCallback, Handler.Callback {

    public volatile long sampledVsyncTimeNs;

    private static final int CREATE_CHOREOGRAPHER = 0;
    private static final int MSG_ADD_OBSERVER = 1;
    private static final int MSG_REMOVE_OBSERVER = 2;

    private static final VSyncSampler INSTANCE = new VSyncSampler();

    private final Handler handler;
    private final HandlerThread choreographerOwnerThread;
    private Choreographer choreographer;
    private int observerCount;

    public static VSyncSampler getInstance() {
      return INSTANCE;
    }

    private VSyncSampler() {
      choreographerOwnerThread = new HandlerThread("ChoreographerOwner:Handler");
      choreographerOwnerThread.start();
      handler = new Handler(choreographerOwnerThread.getLooper(), this);
      handler.sendEmptyMessage(CREATE_CHOREOGRAPHER);
    }

    /**
     * Notifies the sampler that a {@link VideoFrameReleaseTimeHelper} is observing {@link
     * #sampledVsyncTimeNs}, and hence that the value should be periodically updated.
     */
    public void addObserver() {
      handler.sendEmptyMessage(MSG_ADD_OBSERVER);
    }

    /**
     * Notifies the sampler that a {@link VideoFrameReleaseTimeHelper} is no longer observing {@link
     * #sampledVsyncTimeNs}.
     */
    public void removeObserver() {
      handler.sendEmptyMessage(MSG_REMOVE_OBSERVER);
    }

    @Override
    public void doFrame(long vsyncTimeNs) {
      sampledVsyncTimeNs = vsyncTimeNs;
      choreographer.postFrameCallbackDelayed(this, CHOREOGRAPHER_SAMPLE_DELAY_MILLIS);
    }

    @Override
    public boolean handleMessage(Message message) {
      switch (message.what) {
        case CREATE_CHOREOGRAPHER:
          {
            createChoreographerInstanceInternal();
            return true;
          }
        case MSG_ADD_OBSERVER:
          {
            addObserverInternal();
            return true;
          }
        case MSG_REMOVE_OBSERVER:
          {
            removeObserverInternal();
            return true;
          }
        default:
          {
            return false;
          }
      }
    }

    private void createChoreographerInstanceInternal() {
      choreographer = Choreographer.getInstance();
    }

    private void addObserverInternal() {
      observerCount++;
      if (observerCount == 1) {
        choreographer.postFrameCallback(this);
      }
    }

    private void removeObserverInternal() {
      observerCount--;
      if (observerCount == 0) {
        choreographer.removeFrameCallback(this);
        sampledVsyncTimeNs = 0;
      }
    }
  }
}
