/*
 * Copyright (c) 2026 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** 创作台 VM 工具 — 延时与带截止时间的 Promise */
export class StudioVmUtils {
  private static clampMs(ms: number): number {
    return ms < 0 ? 0 : ms;
  }

  public static bridgeDelay(ms: number): Promise<void> {
    const waitMs = StudioVmUtils.clampMs(ms);
    return new Promise(resolve => setTimeout(resolve, waitMs));
  }

  public static async awaitWithDeadline<T>(task: Promise<T>, timeoutMs: number, fallback: T): Promise<T> {
    const waitMs = StudioVmUtils.clampMs(timeoutMs);
    let timerId: number = -1;
    const timeout = new Promise<T>((resolve) => {
      timerId = setTimeout(() => resolve(fallback), waitMs) as number;
    });
    try {
      return await Promise.race([task, timeout]);
    } finally {
      if (timerId >= 0) {
        clearTimeout(timerId);
      }
    }
  }
}

