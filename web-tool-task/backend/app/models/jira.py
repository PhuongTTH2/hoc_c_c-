from sqlalchemy import Column, String, DateTime, JSON, Integer, Text, Date
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.sql import func
from app.core.database import Base
import uuid

class JiraDetails(Base):
    __tablename__ = "jira_details"

    id = Column(UUID(as_uuid=True), primary_key=True, default=uuid.uuid4)
    jira_ticket_key = Column(String(50), unique=True, nullable=False)
    jira_ticket_id = Column(String(50))
    jira_ticket_url = Column(String(500))
    task_name = Column(String(500), nullable=False)
    task_description = Column(Text)
    created_by = Column(String(255))
    assignee_name = Column(String(255))
    start_date = Column(Date, nullable=True)
    due_date = Column(Date, nullable=True)
    priority = Column(String(50), default="Medium")
    issue_type = Column(String(50), default="Task")
    project_key = Column(String(50))
    status = Column(String(50), default="To Do")
    labels = Column(JSON, default=list)
    raw_response = Column(JSON, default=dict)
    created_at = Column(DateTime(timezone=True), server_default=func.now())