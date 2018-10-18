#pragma once

#include "drape/drape_diagnostics.hpp"
#include "drape/utils/gpu_mem_tracker.hpp"
#include "drape/utils/glyph_usage_tracker.hpp"

#include "base/thread.hpp"
#include "base/timer.hpp"

#include <chrono>
#include <map>
#include <memory>
#include <mutex>
#include <numeric>
#include <vector>
#include <unordered_map>

namespace df
{
class DrapeMeasurer
{
public:
  static DrapeMeasurer & Instance();

  void StartBenchmark();
  void StopBenchmark();

#ifdef RENDER_STATISTIC
  struct RenderStatistic
  {
    std::string ToString() const;

    uint32_t m_FPS = 0;
    uint32_t m_minFPS = 0;
    uint32_t m_frameRenderTimeInMs = 0;
    std::map<uint32_t, float> m_fpsDistribution;
    uint32_t m_immediateRenderingFPS = 0;
    uint32_t m_immediateRenderingMinFPS = 0;
  };

  RenderStatistic GetRenderStatistic();
#endif

#ifdef TILES_STATISTIC
  struct TileStatistic
  {
    std::string ToString() const;

    uint32_t m_totalTilesCount = 0;
    uint32_t m_tileReadTimeInMs = 0;
  };

  void StartTileReading();
  void EndTileReading();

  TileStatistic GetTileStatistic();
#endif

#ifdef GENERATING_STATISTIC
  struct GeneratingStatistic
  {
    std::string ToString() const;

    uint32_t m_maxScenePreparingTimeInMs = 0;

    uint32_t m_shapesCount = 0;
    uint32_t m_shapeGenTimeInMs = 0;

    uint32_t m_overlayShapesCount = 0;
    uint32_t m_overlayShapeGenTimeInMs = 0;
  };

  void StartScenePreparing();
  void EndScenePreparing();

  void StartShapesGeneration();
  void EndShapesGeneration(uint32_t shapesCount);

  void StartOverlayShapesGeneration();
  void EndOverlayShapesGeneration(uint32_t shapesCount);

  GeneratingStatistic GetGeneratingStatistic();
#endif

#ifdef TRACK_GPU_MEM
  struct GPUMemoryStatistic
  {
    std::string ToString() const;

    dp::GPUMemTracker::GPUMemorySnapshot m_averageMemoryValues;
    dp::GPUMemTracker::GPUMemorySnapshot m_maxMemoryValues;
  };

  GPUMemoryStatistic GetGPUMemoryStatistic();
#endif

#if defined(RENDER_STATISTIC) || defined(TRACK_GPU_MEM)
  void BeforeRenderFrame();
  void BeforeImmediateRendering();
  void AfterImmediateRendering();
  void AfterRenderFrame();
#endif

  struct DrapeStatistic
  {
    std::string ToString() const;

#ifdef RENDER_STATISTIC
    RenderStatistic m_renderStatistic;
#endif
#ifdef TILES_STATISTIC
    TileStatistic m_tileStatistic;
#endif
#ifdef GENERATING_STATISTIC
    GeneratingStatistic m_generatingStatistic;
#endif
#ifdef TRACK_GPU_MEM
    GPUMemoryStatistic m_gpuMemStatistic;
#endif
#ifdef TRACK_GLYPH_USAGE
    dp::GlyphUsageTracker::GlyphUsageStatistic m_glyphStatistic;
#endif
  };

  DrapeStatistic GetDrapeStatistic();

private:
  DrapeMeasurer() = default;

  bool m_isEnabled = false;

#ifdef GENERATING_STATISTIC
  std::chrono::time_point<std::chrono::steady_clock> m_startScenePreparingTime;
  std::chrono::nanoseconds m_maxScenePreparingTime;

  std::chrono::time_point<std::chrono::steady_clock> m_startShapesGenTime;
  std::chrono::nanoseconds m_totalShapesGenTime;
  uint32_t m_totalShapesCount = 0;

  std::chrono::time_point<std::chrono::steady_clock> m_startOverlayShapesGenTime;
  std::chrono::nanoseconds m_totalOverlayShapesGenTime;
  uint32_t m_totalOverlayShapesCount = 0;
#endif

#ifdef TILES_STATISTIC
  struct TileReadInfo
  {
    std::chrono::time_point<std::chrono::steady_clock> m_startTileReadTime;
    std::chrono::nanoseconds m_totalTileReadTime;
    uint32_t m_totalTilesCount = 0;
  };
  std::map<threads::ThreadID, std::shared_ptr<TileReadInfo>> m_tilesReadInfo;
  std::mutex m_tilesMutex;
#endif

#ifdef RENDER_STATISTIC
  std::chrono::nanoseconds m_totalTPF;
  uint32_t m_totalTPFCount = 0;

  uint32_t m_minFPS = std::numeric_limits<uint32_t>::max();
  double m_totalFPS = 0.0;
  uint32_t m_totalFPSCount = 0;
  std::unordered_map<uint32_t, uint32_t> m_fpsDistribution;

  uint32_t m_immediateRenderingMinFps = std::numeric_limits<uint32_t>::max();
  std::chrono::nanoseconds m_immediateRenderingTimeSum;
  uint64_t m_immediateRenderingFramesCount = 0;
#endif

#if defined(RENDER_STATISTIC) || defined(TRACK_GPU_MEM)
  std::chrono::time_point<std::chrono::steady_clock> m_startFrameRenderTime;
  std::chrono::time_point<std::chrono::steady_clock> m_startImmediateRenderingTime;
  std::chrono::nanoseconds m_totalFrameRenderTime;
  uint32_t m_totalFramesCount = 0;
#endif

#ifdef TRACK_GPU_MEM
  void TakeGPUMemorySnapshot();

  dp::GPUMemTracker::GPUMemorySnapshot m_maxSnapshotValues;
  dp::GPUMemTracker::GPUMemorySnapshot m_summarySnapshotValues;
  uint32_t m_numberOfSnapshots = 0;
#endif
};
}  // namespace df
