#include "critics_viz_rviz/critics_panel.hpp"

#include <algorithm>
#include <cmath>
#include <sstream>
#include <iomanip>

#include <pluginlib/class_list_macros.hpp>

PLUGINLIB_EXPORT_CLASS(critics_viz_rviz::CriticsPanel, rviz_common::Panel)

namespace critics_viz_rviz
{

// ── Color palette ─────────────────────────────────────────────────────────────
const std::vector<QColor> CriticsPanel::COLORS = {
  QColor(0x37,0x8A,0xDD),
  QColor(0x1D,0x9E,0x75),
  QColor(0xD8,0x5A,0x30),
  QColor(0xD4,0x53,0x7E),
  QColor(0x7F,0x77,0xDD),
  QColor(0xBA,0x75,0x17),
  QColor(0xE2,0x4B,0x4A),
  QColor(0x88,0x87,0x80),
  QColor(0x0F,0x6E,0x56),
  QColor(0x99,0x3C,0x1D),
  QColor(0x3B,0x6D,0x11),
  QColor(0x63,0x38,0x06),
};

// ── MetricCard ────────────────────────────────────────────────────────────────
MetricCard::MetricCard(const QString & label, QWidget * parent)
: QFrame(parent)
{
  setFrameShape(QFrame::StyledPanel);
  setStyleSheet("MetricCard{background:#1e2330;border-radius:8px;border:none;}");
  auto * l = new QVBoxLayout(this);
  l->setContentsMargins(10,8,10,8);
  l->setSpacing(2);
  label_ = new QLabel(label, this);
  label_->setStyleSheet("font-size:11px;color:#64748b;");
  value_ = new QLabel("—", this);
  value_->setStyleSheet("font-size:18px;font-weight:500;color:#e2e8f0;");
  sub_ = new QLabel("", this);
  sub_->setStyleSheet("font-size:10px;color:#475569;");
  l->addWidget(label_);
  l->addWidget(value_);
  l->addWidget(sub_);
}

void MetricCard::setValue(const QString & val, const QString & sub)
{
  value_->setText(val);
  sub_->setText(sub);
  sub_->setVisible(!sub.isEmpty());
}

// ── CriticBarRow ──────────────────────────────────────────────────────────────
CriticBarRow::CriticBarRow(const QString & name, const QColor & color,
                           QWidget * parent)
: QWidget(parent), color_(color)
{
  auto * l = new QHBoxLayout(this);
  l->setContentsMargins(0,2,0,2);
  l->setSpacing(6);

  name_label_ = new QLabel(name, this);
  name_label_->setFixedWidth(120);
  name_label_->setStyleSheet("font-size:12px;color:#94a3b8;");
  name_label_->setToolTip(name + "Critic");

  dot_ = new QFrame(this);
  dot_->setFixedSize(8,8);
  dot_->setStyleSheet("background:#334155;border-radius:4px;");

  bar_ = new QProgressBar(this);
  bar_->setRange(0,1000);
  bar_->setValue(0);
  bar_->setTextVisible(false);
  bar_->setFixedHeight(9);
  bar_->setStyleSheet(QString(
    "QProgressBar{background:#0f1117;border-radius:4px;border:none;}"
    "QProgressBar::chunk{background:%1;border-radius:4px;}").arg(color.name()));

  val_label_ = new QLabel("0.00", this);
  val_label_->setFixedWidth(80);
  val_label_->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  val_label_->setStyleSheet("font-size:11px;font-family:monospace;color:#e2e8f0;");

  l->addWidget(name_label_);
  l->addWidget(dot_);
  l->addWidget(bar_, 1);
  l->addWidget(val_label_);
}

void CriticBarRow::update(double cost, double max_cost, double pct, bool triggered)
{
  bar_->setValue(max_cost > 0
    ? static_cast<int>((cost / max_cost) * 1000.0) : 0);
  std::ostringstream ss;
  ss << std::fixed << std::setprecision(2) << cost
     << " (" << std::setprecision(1) << pct << "%)";
  val_label_->setText(QString::fromStdString(ss.str()));
  dot_->setStyleSheet(triggered
    ? "background:#4ade80;border-radius:4px;"
    : "background:#334155;border-radius:4px;");
}

// ── HistoryChart ──────────────────────────────────────────────────────────────
HistoryChart::HistoryChart(QWidget * parent)
: QChartView(parent)
{
  chart_ = new QChart();
  chart_->setBackgroundBrush(QBrush(QColor(0x1e,0x23,0x30)));
  chart_->setPlotAreaBackgroundBrush(QBrush(QColor(0x0f,0x11,0x17)));
  chart_->setPlotAreaBackgroundVisible(true);
  chart_->setMargins(QMargins(4,4,4,4));
  chart_->legend()->setVisible(false);

  axis_x_ = new QValueAxis();
  axis_x_->setRange(0, MAX_HISTORY);
  axis_x_->setLabelsVisible(false);
  axis_x_->setGridLineColor(QColor(0x33,0x41,0x55));

  axis_y_ = new QValueAxis();
  axis_y_->setRange(0, 10);
  axis_y_->setLabelsColor(QColor(0x47,0x56,0x69));
  axis_y_->setGridLineColor(QColor(0x33,0x41,0x55,80));
  axis_y_->setTickCount(5);
  axis_y_->setLabelFormat("%.1f");

  chart_->addAxis(axis_x_, Qt::AlignBottom);
  chart_->addAxis(axis_y_, Qt::AlignLeft);
  setChart(chart_);
  setRenderHint(QPainter::Antialiasing);
  setStyleSheet("background:transparent;border:none;");
}

void HistoryChart::ensureSeries(const std::vector<std::string> & names,
                                const std::vector<QColor> & colors)
{
  if (names == critic_names_) return;
  for (auto * s : series_) { chart_->removeSeries(s); delete s; }
  series_.clear();
  critic_names_ = names;
  frame_count_  = 0;
  for (size_t i = 0; i < names.size(); i++) {
    auto * s = new QLineSeries();
    QPen pen(colors[i % colors.size()]);
    pen.setWidthF(1.5);
    s->setPen(pen);
    chart_->addSeries(s);
    s->attachAxis(axis_x_);
    s->attachAxis(axis_y_);
    series_.push_back(s);
  }
}

void HistoryChart::addFrame(const std::vector<double> & costs)
{
  if (costs.size() != series_.size()) return;
  double max_y = 1.0;
  for (size_t i = 0; i < series_.size(); i++) {
    series_[i]->append(frame_count_, costs[i]);
    max_y = std::max(max_y, costs[i]);
    if (series_[i]->count() > MAX_HISTORY) series_[i]->remove(0);
  }
  int x_end = std::max(frame_count_, MAX_HISTORY);
  axis_x_->setRange(x_end - MAX_HISTORY, x_end);
  axis_y_->setRange(0, max_y * 1.1);
  frame_count_++;
}

// ── CriticsPanel ──────────────────────────────────────────────────────────────
CriticsPanel::CriticsPanel(QWidget * parent)
: rviz_common::Panel(parent)
{
  buildUI();
}

CriticsPanel::~CriticsPanel()
{
  if (timer_) timer_->stop();

  // Reset subscription before the node reference is dropped.
  // Do NOT touch the executor — this node belongs to RViz2's executor.
  sub_.reset();
  node_.reset();
}

void CriticsPanel::onInitialize()
{
  // ── get the node that RViz2 already owns and spins ───────────────────────
  // IMPORTANT: do NOT add this node to any other executor — RViz2 already
  // manages it in its own executor.  Doing so causes a std::runtime_error
  // ("Node has already been added to an executor") that kills the process.
  auto ros_node_abstraction = getDisplayContext()->getRosNodeAbstraction().lock();
  node_ = ros_node_abstraction->get_raw_node();

  // ── create subscription — callbacks dispatched by RViz2's executor ────────
  sub_ = node_->create_subscription<nav2_msgs::msg::CriticsStats>(
    "/controller_server/critics_stats",
    rclcpp::SensorDataQoS(),
    [this](nav2_msgs::msg::CriticsStats::SharedPtr msg) {
      std::lock_guard<std::mutex> lock(msg_mutex_);
      latest_msg_ = msg;
      new_msg_    = true;
    }
  );

  // ── Qt refresh timer — polls latest_msg_ and updates UI ──────────────────
  timer_ = new QTimer(this);
  connect(timer_, &QTimer::timeout, this, &CriticsPanel::onTimer);
  timer_->start(100);

  status_label_->setText("● waiting for /controller_server/critics_stats");
  status_label_->setStyleSheet("font-size:11px;color:#64748b;");
}

void CriticsPanel::buildUI()
{
  auto * root = new QVBoxLayout(this);
  root->setContentsMargins(8,8,8,8);
  root->setSpacing(8);

  status_label_ = new QLabel("● initialising…", this);
  status_label_->setStyleSheet("font-size:11px;color:#64748b;");
  root->addWidget(status_label_);

  alert_label_ = new QLabel("", this);
  alert_label_->setWordWrap(true);
  alert_label_->setVisible(false);
  root->addWidget(alert_label_);

  auto * cards = new QHBoxLayout();
  cards->setSpacing(8);
  card_active_    = new MetricCard("Active critics",  this);
  card_triggered_ = new MetricCard("Triggered",       this);
  card_dominant_  = new MetricCard("Dominant critic", this);
  card_total_     = new MetricCard("Total cost",      this);
  cards->addWidget(card_active_);
  cards->addWidget(card_triggered_);
  cards->addWidget(card_dominant_);
  cards->addWidget(card_total_);
  root->addLayout(cards);

  auto * sep1 = new QFrame(this);
  sep1->setFrameShape(QFrame::HLine);
  sep1->setStyleSheet("color:#1e2330;");
  root->addWidget(sep1);

  auto * scroll = new QScrollArea(this);
  scroll->setWidgetResizable(true);
  scroll->setMaximumHeight(240);
  scroll->setStyleSheet(
    "QScrollArea{border:none;background:transparent;}"
    "QScrollBar:vertical{width:6px;background:#0f1117;}"
    "QScrollBar::handle:vertical{background:#334155;border-radius:3px;}");

  bars_container_ = new QWidget();
  bars_container_->setStyleSheet("background:transparent;");
  bars_layout_ = new QVBoxLayout(bars_container_);
  bars_layout_->setContentsMargins(0,0,0,0);
  bars_layout_->setSpacing(2);
  scroll->setWidget(bars_container_);
  root->addWidget(scroll);

  auto * sep2 = new QFrame(this);
  sep2->setFrameShape(QFrame::HLine);
  sep2->setStyleSheet("color:#1e2330;");
  root->addWidget(sep2);

  auto * hist_lbl = new QLabel("Cost history — last 80 cycles", this);
  hist_lbl->setStyleSheet("font-size:10px;color:#64748b;");
  root->addWidget(hist_lbl);

  history_chart_ = new HistoryChart(this);
  history_chart_->setMinimumHeight(160);
  root->addWidget(history_chart_);

  setLayout(root);
  setStyleSheet("CriticsPanel{background:#0f1117;}");
}

void CriticsPanel::rebuildCriticRows(const std::vector<std::string> & names)
{
  bar_rows_.clear();
  while (bars_layout_->count()) {
    auto * item = bars_layout_->takeAt(0);
    if (item->widget()) delete item->widget();
    delete item;
  }
  for (size_t i = 0; i < names.size(); i++) {
    QString sname = QString::fromStdString(names[i]);
    sname.remove("Critic");
    auto row = std::make_unique<CriticBarRow>(sname, COLORS[i % COLORS.size()],
                                              bars_container_);
    bars_layout_->addWidget(row.get());
    bar_rows_.push_back(std::move(row));
  }
  current_critics_ = names;
  history_chart_->ensureSeries(names, COLORS);
}

void CriticsPanel::onTimer()
{
  nav2_msgs::msg::CriticsStats::SharedPtr msg;
  {
    std::lock_guard<std::mutex> lock(msg_mutex_);
    if (!new_msg_) return;
    msg      = latest_msg_;
    new_msg_ = false;
  }
  applyUpdate(*msg);
}

void CriticsPanel::applyUpdate(const nav2_msgs::msg::CriticsStats & msg)
{
  const auto & names    = msg.critics_names;
  const auto & costs    = msg.costs_sum;
  const auto & triggered= msg.critics_triggered;
  if (names.empty()) return;

  std::vector<std::string> name_vec(names.begin(), names.end());
  if (name_vec != current_critics_) rebuildCriticRows(name_vec);

  double total = 0.0, max_c = 0.0;
  for (auto c : costs) { total += c; max_c = std::max(max_c, (double)c); }

  int trig_count = 0;
  for (auto t : triggered) trig_count += t ? 1 : 0;

  int dom_idx = 0;
  for (size_t i = 1; i < costs.size(); i++)
    if (costs[i] > costs[dom_idx]) dom_idx = static_cast<int>(i);
  double dom_pct = total > 0 ? (costs[dom_idx] / total * 100.0) : 0.0;

  card_active_->setValue(QString::number(names.size()));
  card_triggered_->setValue(
    QString("%1 / %2").arg(trig_count).arg((int)names.size()));
  QString dom_name = QString::fromStdString(names[dom_idx]);
  dom_name.remove("Critic");
  card_dominant_->setValue(dom_name,
    QString("%1% of total").arg(dom_pct, 0, 'f', 1));
  card_total_->setValue(QString::number(total, 'f', 2));

  status_label_->setText("● live  —  /controller_server/critics_stats");
  status_label_->setStyleSheet("font-size:11px;color:#4ade80;");

  // alert
  int obs_idx = -1;
  for (size_t i = 0; i < names.size(); i++)
    if (std::string(names[i]).find("Obstacle") != std::string::npos)
      { obs_idx = static_cast<int>(i); break; }

  if (obs_idx >= 0 && costs[obs_idx] > 20.0) {
    alert_label_->setText(
      QString("⚠  ObstaclesCritic cost high (%1) — near obstacle.")
      .arg((double)costs[obs_idx], 0, 'f', 1));
    alert_label_->setStyleSheet(
      "font-size:11px;padding:5px 10px;border-radius:5px;"
      "border-left:3px solid #f59e0b;background:#2a1a08;color:#fbbf24;");
    alert_label_->setVisible(true);
  } else if (total < 1.0) {
    alert_label_->setText("✓  All costs near zero — path followed well.");
    alert_label_->setStyleSheet(
      "font-size:11px;padding:5px 10px;border-radius:5px;"
      "border-left:3px solid #3b82f6;background:#0a1e30;color:#60a5fa;");
    alert_label_->setVisible(true);
  } else {
    alert_label_->setVisible(false);
  }

  for (size_t i = 0; i < bar_rows_.size() && i < costs.size(); i++) {
    double pct = total > 0 ? (costs[i] / total * 100.0) : 0.0;
    bar_rows_[i]->update(costs[i], max_c, pct, triggered[i]);
  }

  std::vector<double> cv(costs.begin(), costs.end());
  history_chart_->addFrame(cv);
}

void CriticsPanel::save(rviz_common::Config config) const
{
  rviz_common::Panel::save(config);
}

void CriticsPanel::load(const rviz_common::Config & config)
{
  rviz_common::Panel::load(config);
}

}  // namespace critics_viz_rviz
