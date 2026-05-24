#ifndef CRITICS_VIZ_RVIZ__CRITICS_PANEL_HPP_
#define CRITICS_VIZ_RVIZ__CRITICS_PANEL_HPP_

#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include <QWidget>
#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QProgressBar>
#include <QFrame>
#include <QTimer>
#include <QScrollArea>
#include <QPainter>

#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QtCharts/QLegend>

#include <rclcpp/rclcpp.hpp>
#include <rviz_common/panel.hpp>
#include <rviz_common/display_context.hpp>
#include <nav2_msgs/msg/critics_stats.hpp>

QT_CHARTS_USE_NAMESPACE

namespace critics_viz_rviz
{

// ── Metric card ───────────────────────────────────────────────────────────────
class MetricCard : public QFrame
{
  Q_OBJECT
public:
  explicit MetricCard(const QString & label, QWidget * parent = nullptr);
  void setValue(const QString & val, const QString & sub = "");
private:
  QLabel * label_;
  QLabel * value_;
  QLabel * sub_;
};

// ── Single critic bar row ─────────────────────────────────────────────────────
class CriticBarRow : public QWidget
{
  Q_OBJECT
public:
  explicit CriticBarRow(const QString & name, const QColor & color,
                        QWidget * parent = nullptr);
  void update(double cost, double max_cost, double pct, bool triggered);
private:
  QLabel       * name_label_;
  QFrame       * dot_;
  QProgressBar * bar_;
  QLabel       * val_label_;
  QColor         color_;
};

// ── History chart ─────────────────────────────────────────────────────────────
class HistoryChart : public QChartView
{
  Q_OBJECT
public:
  static constexpr int MAX_HISTORY = 80;
  explicit HistoryChart(QWidget * parent = nullptr);
  void ensureSeries(const std::vector<std::string> & names,
                    const std::vector<QColor> & colors);
  void addFrame(const std::vector<double> & costs);

private:
  QChart     * chart_;
  QValueAxis * axis_x_;
  QValueAxis * axis_y_;
  std::vector<QLineSeries *> series_;
  std::vector<std::string>   critic_names_;
  int frame_count_{0};
};

// ── Main panel ────────────────────────────────────────────────────────────────
class CriticsPanel : public rviz_common::Panel
{
  Q_OBJECT
public:
  explicit CriticsPanel(QWidget * parent = nullptr);
  ~CriticsPanel() override;

  void onInitialize() override;
  void save(rviz_common::Config config) const override;
  void load(const rviz_common::Config & config) override;

private Q_SLOTS:
  void onTimer();

private:
  void buildUI();
  void rebuildCriticRows(const std::vector<std::string> & names);
  void applyUpdate(const nav2_msgs::msg::CriticsStats & msg);

  // ROS — use raw node ptr from RViz2 context (no ownership).
  // RViz2 already spins this node; do NOT add it to another executor.
  rclcpp::Node::SharedPtr node_;
  rclcpp::Subscription<nav2_msgs::msg::CriticsStats>::SharedPtr sub_;

  // thread-safe msg handoff
  std::mutex                                  msg_mutex_;
  nav2_msgs::msg::CriticsStats::SharedPtr     latest_msg_;
  bool                                        new_msg_{false};

  // UI
  QLabel       * status_label_;
  QLabel       * alert_label_;
  MetricCard   * card_active_;
  MetricCard   * card_triggered_;
  MetricCard   * card_dominant_;
  MetricCard   * card_total_;
  QWidget      * bars_container_;
  QVBoxLayout  * bars_layout_;
  HistoryChart * history_chart_;
  QTimer       * timer_;

  std::vector<std::unique_ptr<CriticBarRow>> bar_rows_;
  std::vector<std::string> current_critics_;

  static const std::vector<QColor> COLORS;
};

}  // namespace critics_viz_rviz

#endif  // CRITICS_VIZ_RVIZ__CRITICS_PANEL_HPP_
